/*
 * Simple Socket Demo
 * 
 * Single file example showing server and client usage
 * Compile: g++ simple_socket_demo.cpp -o simple_socket_demo
 * 
 * Usage:
 *   Server: ./simple_socket_demo server <port>
 *   Client: ./simple_socket_demo client <port>
 */

 #include "simple_socket.h"
 #include "stdio.h"
 #include "string.h"
 #include "stdlib.h"
 
 #define DEFAULT_PORT 8888
 #define MAX_MESSAGE_LENGTH 1024
 void runServer(int port) {
     printf("Starting server on port %d...\n", port);
     
     SOCKET serverFd = SimpleSocket::createServer(port);
     if (serverFd == INVALID_SOCKET) {
         printf("Failed to create server\n");
         return;
     }
     
     printf("Server listening, waiting for client...\n");
     
     while (1) {
         SOCKET clientFd = SimpleSocket::acceptClient(serverFd);
         if (clientFd == INVALID_SOCKET) {
             printf("Accept failed\n");
             continue;
         }
         
         printf("Client connected\n");
         
         // Loop to handle multiple messages from the same client
         while (1) {
             // Echo server: receive length first, then data
             uint32_t msgLen = 0;
             if (SimpleSocket::recvAll(clientFd, (uint8_t*)&msgLen, 4) != 4) {
                 printf("Failed to receive message length (client may have disconnected)\n");
                 break;
             }
             
             if (msgLen > MAX_MESSAGE_LENGTH) {
                 printf("Message too long: %d\n", msgLen);
                 break;
             }
             
             char buffer[MAX_MESSAGE_LENGTH];
             int received = SimpleSocket::recvAll(clientFd, (uint8_t*)buffer, msgLen);
             if (received <= 0) {
                 printf("Failed to receive message data (client may have disconnected)\n");
                 break;
             }
             
             buffer[received] = '\0';
             printf("Received (%d bytes): %s\n", received, buffer);
             
             // Send back: length + data
             if (SimpleSocket::sendAll(clientFd, (uint8_t*)&msgLen, 4) != 4) {
                 printf("Failed to send echo length\n");
                 break;
             }
             if (SimpleSocket::sendAll(clientFd, (uint8_t*)buffer, received) != received) {
                 printf("Failed to send echo data\n");
                 break;
             }
             printf("Echoed back\n");
         }
         
         closesocket(clientFd);
         printf("Client disconnected\n");
     }
     
     closesocket(serverFd);
 }
 
 void runClient(int port) {
    printf("Connecting to server on port %d...\n", port);
    
    SOCKET clientFd = SimpleSocket::createClient("127.0.0.1", port);
    if (clientFd == INVALID_SOCKET) {
        printf("Failed to connect to server\n");
        return;
    }
    
    printf("Connected to server\n");
    printf("Enter messages (type 'quit' to exit):\n");
    
    char inputBuffer[MAX_MESSAGE_LENGTH];
    char echoBuffer[MAX_MESSAGE_LENGTH];
     
    while (1) {
        // Read from stdin
        printf("> ");
        fflush(stdout);
        
        if (fgets(inputBuffer, MAX_MESSAGE_LENGTH, stdin) == NULL) {
            printf("\nEOF or error reading input\n");
            break;
        }
        
        // Remove newline
        size_t len = strlen(inputBuffer);
        if (len > 0 && inputBuffer[len - 1] == '\n') {
            inputBuffer[len - 1] = '\0';
            len--;
        }
        
        // Check for quit command
        if (strcmp(inputBuffer, "quit") == 0 || strcmp(inputBuffer, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        
        if (len == 0) {
            continue; // Skip empty input
        }
        
        // Send message: length first, then data
        uint32_t msgLen = len;
        printf("Sending (%d bytes): %s\n", msgLen, inputBuffer);
        
        // Send length
        if (SimpleSocket::sendAll(clientFd, (uint8_t*)&msgLen, 4) != 4) {
            printf("Failed to send message length\n");
            break;
        }
        
        // Send data
        if (SimpleSocket::sendAll(clientFd, (const uint8_t*)inputBuffer, msgLen) != msgLen) {
            printf("Failed to send message data\n");
            break;
        }
        
        // Receive echo: length first, then data
        uint32_t echoLen = 0;
        if (SimpleSocket::recvAll(clientFd, (uint8_t*)&echoLen, 4) != 4) {
            printf("Failed to receive echo length\n");
            break;
        }
        
        if (echoLen > MAX_MESSAGE_LENGTH) {
            printf("Echo message too long: %d\n", echoLen);
            break;
        }
        
        int received = SimpleSocket::recvAll(clientFd, (uint8_t*)echoBuffer, echoLen);
        if (received > 0) {
            echoBuffer[received] = '\0';
            printf("Echo (%d bytes): %s\n", received, echoBuffer);
        } else {
            printf("Failed to receive echo data\n");
            break;
        }
    }
    
    closesocket(clientFd);
    printf("Disconnected\n");
 }
 
 int main_socket_demo(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server|client> <--port <port>>\n", argv[0]);
        printf("Example: %s server --port %d\n", argv[0], DEFAULT_PORT);
        printf("         %s client --port %d\n", argv[0], DEFAULT_PORT);
        return 1;
    }
    int port = DEFAULT_PORT;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
            break;
        }
    }

    if (port <= 0 || port > 65535) {
        printf("Invalid port: %d\n", port);
        return 1;
    }

    if (!SimpleSocket::init()) {
        printf("Failed to initialize socket library\n");
        return 1;
    }
     
    if (strcmp(argv[1], "server") == 0) {
        runServer(port);
    } else if (strcmp(argv[1], "client") == 0) {
        runClient(port);
    } else {
        printf("Invalid mode: %s (use 'server' or 'client')\n", argv[1]);
        return 1;
    }
     
    SimpleSocket::cleanup();
    return 0;
 }
 