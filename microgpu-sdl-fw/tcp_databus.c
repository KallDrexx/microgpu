#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "microgpu-common/databus.h"
#include "tcp_databus.h"

int sockInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
#else
    return 0;
#endif
}

int sockQuit(void)
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

int sockClose(SOCKET sock)
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

Mgpu_Databus *mgpu_databus_init(Mgpu_DatabusOptions *options, Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    Mgpu_Databus *databus = allocator->Mgpu_AllocateFn(sizeof(Mgpu_Databus));
    if (databus == NULL) {
        return NULL;
    }

    // create the socket
    sockInit();
    databus->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (isInvalidSocket(databus->socket)) {
        fprintf(stderr, "Invalid socket returned\n");
        return NULL;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = options->port;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    if (bind(databus->socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        fprintf(stderr, "Failed to bind socket\n");
        return NULL;
    }

    if ((listen(databus->socket, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }


}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus != NULL) {
        sockClose(databus->socket);
        sockQuit();
        databus->allocator->Mgpu_FreeFn(databus);
    }
}