#include "TcpServer.hpp"
#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <arpa/inet.h>

TcpServer::TcpServer(int port) : port(port), serverSocket(-1) {}

void TcpServer::setupServerSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::error("Failed to create server socket");
        throw std::runtime_error("Failed to create server socket");
    }

    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        Logger::error("setsockopt failed");
        throw std::runtime_error("setsockopt failed");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        Logger::error("Failed to bind to port " + std::to_string(port));
        throw std::runtime_error("Failed to bind to port " + std::to_string(port));
    }

    if (listen(serverSocket, 5) != 0) {
        Logger::error("listen failed");
        throw std::runtime_error("listen failed");
    }
}

void* clientThreadFunction(void* arg) {
    int clientSocket = *((int*)arg);
    delete (int*)arg;
    
    ClientHandler handler;
    handler.handleClient(clientSocket);
    
    return nullptr;
}

void TcpServer::start() {
    setupServerSocket();
    Logger::log("Server listening on port " + std::to_string(port));

    // Load the RDB file
    ClientHandler::loadRdbFile();

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket < 0) {
            Logger::error("accept failed");
            continue;
        }

        int* clientSocketPtr = new int(clientSocket);
        pthread_t thread;
        if (pthread_create(&thread, nullptr, clientThreadFunction, clientSocketPtr) != 0) {
            Logger::error("Failed to create thread");
            close(clientSocket);
            delete clientSocketPtr;
        } else {
            Logger::connection(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), "Connected");
            pthread_detach(thread);
        }
    }
}