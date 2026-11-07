#ifndef _SIMPLE_SOCKET_H_
#define _SIMPLE_SOCKET_H_

#include "stdint.h"

#ifdef _WIN32
// Avoid winsock.h being pulled in before winsock2.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

class SimpleSocket {
public:
    static bool init() {
#ifdef _WIN32
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
        return true;
#endif
    }
    
    static void cleanup() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    static SOCKET createServer(int port) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return INVALID_SOCKET;
        
        int opt = 1;
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(sock);
            return INVALID_SOCKET;
        }
        
        if (listen(sock, 1) == SOCKET_ERROR) {
            closesocket(sock);
            return INVALID_SOCKET;
        }
        
        return sock;
    }
    
    static SOCKET acceptClient(SOCKET serverSock) {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        return accept(serverSock, (struct sockaddr*)&clientAddr, &len);
    }
    
    static SOCKET createClient(const char* host, int port) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) return INVALID_SOCKET;
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, host, &addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(sock);
            return INVALID_SOCKET;
        }
        
        return sock;
    }
    
    static int sendAll(SOCKET sock, const uint8_t* data, int len) {
        int sent = 0;
        while (sent < len) {
            int n = send(sock, (const char*)(data + sent), len - sent, 0);
            if (n <= 0) return -1;
            sent += n;
        }
        return sent;
    }
    
    static int recvAll(SOCKET sock, uint8_t* data, int len) {
        int received = 0;
        while (received < len) {
            int n = recv(sock, (char*)(data + received), len - received, 0);
            if (n <= 0) return -1;
            received += n;
        }
        return received;
    }
};

#endif
