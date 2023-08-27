#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <SDL.h>
#include "microgpu-common/databus.h"
#include "tcp_databus.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

int initSockets(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
#else
    return 0;
#endif
}

int quitSocketHandling(void)
{
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

int closeSocket(SOCKET sock)
{
    int status = 0;

#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) { status = closesocket(sock); }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) { status = close(sock); }
#endif

    return status;
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    Mgpu_Databus *databus = allocator->Mgpu_AllocateFn(sizeof(Mgpu_Databus));
    if (databus == NULL) {
        return NULL;
    }

    databus->serverSocket = INVALID_SOCKET;
    databus->clientSocket = INVALID_SOCKET;

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
    serverAddr.sin_port = options->port;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

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
        databus->allocator->Mgpu_FreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(databus != NULL);
    assert(operation != NULL);

    if (isInvalidSocket(databus->clientSocket)) {
        struct sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);

        databus->clientSocket = accept(databus->serverSocket, (struct sockaddr *) &clientAddr, &addrSize);
        if (isInvalidSocket(databus->clientSocket)) {
            fprintf(stderr, "Failed to accept client socket\n");
            return false;
        }
    }

    char buffer[1024];
    int bytesRead = recv(databus->clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        fprintf(stderr, "Failed to read from client socket\n");
        databus->clientSocket = INVALID_SOCKET;
        return false;
    }

    if (bytesRead == 0) {
        SDL_Log("Client disconnected\n");
        databus->clientSocket = INVALID_SOCKET;
        return false;
    }

    return false;
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    // TODO: fill in
}
