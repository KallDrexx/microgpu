#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <SDL.h>
#include "microgpu-common/databus.h"
#include "microgpu-common/operations/operation_deserializer.h"
#include "microgpu-common/responses/response_serializer.h"
#include "microgpu-common/packet_framing.h"
#include "tcp_databus.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#define BYTE_QUEUE_MAX_SIZE 20000
uint8_t *globalByteQueue;
size_t globalByteQueueSize; // How many bytes we've pushed into the byte queue
uint8_t operationBytes[1024];

int initSockets(void) {
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
    return 0;
#endif
}

int quitSocketHandling(void) {
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

bool isInvalidSocket(SOCKET socket) {
#ifdef _WIN32
    return socket == INVALID_SOCKET;
#else
    return socket < 0;
#endif
}

int closeSocket(SOCKET sock) {
    int status;

#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) { status = closesocket(sock); }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { status = close(sock); }
#endif

    return status;
}

bool readBytes(Mgpu_Databus *databus, char *buffer, size_t bufferSize, int *bytesRead) {
    *bytesRead = recv(databus->clientSocket, buffer, (int) bufferSize, 0);
    if (*bytesRead == -1) {
        fprintf(stderr, "Failed to read from client socket\n");
        closeSocket(databus->clientSocket);
        databus->clientSocket = INVALID_SOCKET;
        return false;
    }

    if (*bytesRead == 0) {
        SDL_Log("Client disconnected\n");
        closeSocket(databus->clientSocket);
        databus->clientSocket = INVALID_SOCKET;
        return false;
    }

    // Connection is good and we received some bytes.
    return true;
}

void readPacketFromQueue(Mgpu_Operation *operation, bool *hadFullPacket, bool *operationDeserializeResult) {
    *hadFullPacket = false;
    *operationDeserializeResult = false;

    size_t decodedByteCount, inputBytesProcessed;
    mgpu_packet_framing_decode(globalByteQueue,
                               globalByteQueueSize,
                               (uint8_t *)&operationBytes,
                               sizeof(operationBytes),
                               &decodedByteCount,
                               &inputBytesProcessed);

    if (inputBytesProcessed == 0) {
        // Could not find a complete packet
        if (globalByteQueueSize >= BYTE_QUEUE_MAX_SIZE) {
            // We have a full queue but no valid packet.
            fprintf(stderr, "TCP queue was full but not one valid packet could be found. Most likely corrupted data");
            globalByteQueueSize = 0;
        }

        return;
    }

    size_t bytesToShift = globalByteQueueSize - inputBytesProcessed;
    if (bytesToShift > 0) {
        memmove(globalByteQueue, globalByteQueue + inputBytesProcessed, bytesToShift);
    }

    *hadFullPacket = true;
    globalByteQueueSize = bytesToShift;
    if (decodedByteCount == 0) {
        // decoding failed
        return;
    }

    *operationDeserializeResult = mgpu_operation_deserialize(operationBytes, decodedByteCount, operation);
}

bool readOperation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    char buffer[1024];

    while (true) {
        bool hasFullPacket, operationDeserializationResult;
        readPacketFromQueue(operation, &hasFullPacket, &operationDeserializationResult);

        if (hasFullPacket) {
            return operationDeserializationResult;
        }

        // We didn't have a full packet, so we need more bytes
        int bytesRead;
        if (!readBytes(databus, buffer, sizeof(buffer), &bytesRead)) {
            // Reading the connection failed
            return false;
        }

        if (globalByteQueueSize + bytesRead > BYTE_QUEUE_MAX_SIZE) {
            SDL_LogError(0, "Queue grew too large\n");
            exit(2);
        }

        for (int x = 0; x < bytesRead; x++) {
            globalByteQueue[globalByteQueueSize + x] = buffer[x];
        }
        globalByteQueueSize += bytesRead;
    }
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    mgpu_alloc_assert(allocator);

    Mgpu_Databus *databus = allocator->FastMemAllocateFn(sizeof(Mgpu_Databus));
    if (databus == NULL) {
        return NULL;
    }

    databus->allocator = allocator;
    databus->serverSocket = INVALID_SOCKET;
    databus->clientSocket = INVALID_SOCKET;

    // Initialize the packet if it's not already allocated
    if (globalByteQueue == NULL) {
        globalByteQueue = allocator->FastMemAllocateFn(sizeof(uint8_t) * BYTE_QUEUE_MAX_SIZE);
    }

    // create the socket
    initSockets();
    databus->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (isInvalidSocket(databus->serverSocket)) {
        fprintf(stderr, "Invalid socket returned\n");
        mgpu_databus_free(databus);
        return NULL;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(options->port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    SDL_Log("Binding databus to tcp port %u\n", options->port);
    if (bind(databus->serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0) {
        fprintf(stderr, "Failed to bind socket\n");
        mgpu_databus_free(databus);
        return NULL;
    }

    // Only let one connection in at a time
    if ((listen(databus->serverSocket, 1)) != 0) {
        fprintf(stderr, "Listen failed...\n");
        mgpu_databus_free(databus);
        return NULL;
    }

    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus != NULL) {
        closeSocket(databus->clientSocket);
        closeSocket(databus->serverSocket);
        quitSocketHandling();
        databus->allocator->FastMemFreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(databus != NULL);
    assert(operation != NULL);
    assert(globalByteQueue != NULL);

    memset(operation, 0, sizeof(Mgpu_Operation));
    if (isInvalidSocket(databus->clientSocket)) {
        struct sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);

        databus->clientSocket = accept(databus->serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
        SDL_Log("Client connection accepted.\n");
        if (isInvalidSocket(databus->clientSocket)) {
            fprintf(stderr, "Failed to accept client socket\n");
            return false;
        }

        // Clear the byte queue
        globalByteQueueSize = 0;
    }

    return readOperation(databus, operation);
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    uint8_t messageBuffer[1024] = {0};
    int messageBytesWritten = mgpu_serialize_response(response, messageBuffer, sizeof(messageBuffer));
    if (messageBytesWritten < 0) {
        fprintf(stderr, "Failed to serialize response: %u\n", messageBytesWritten);
        return;
    }

    uint8_t outputBuffer[255] = {0};
    int outputBytesWritten = mgpu_packet_framing_encode(messageBuffer,
                                                        messageBytesWritten,
                                                        outputBuffer,
                                                        sizeof(outputBuffer));

    if (outputBytesWritten >= 0) {
        send(databus->clientSocket, (char *) outputBuffer, outputBytesWritten, 0);
    } else {
        SDL_Log("Failed to send response of type %u with error code %u", response->type, outputBytesWritten);
    }
}

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {
    assert(databus != NULL);
    return 1024;
}

