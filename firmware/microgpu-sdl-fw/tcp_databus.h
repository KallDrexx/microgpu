#pragma once

#include <stdint.h>
#include "microgpu-common/alloc.h"

#ifdef _WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32")
#else
// Assume that any non-Windows platform uses POSIX-style sockets instead.

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  // Needed for getaddrinfo() and freeaddrinfo()
#include <unistd.h> // Needed for close()

typedef int SOCKET;
#endif

struct Mgpu_DatabusOptions {
    uint16_t port;
};

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
    SOCKET serverSocket, clientSocket;
};
