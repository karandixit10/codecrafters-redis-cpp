#include "TcpServer.hpp"
#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <pthread.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>

TcpServer::TcpServer(int port, bool isMaster, const std::string& masterHostAndPort)
    : port(port), serverSocket(-1), isMaster(isMaster) {
    Logger::log("Server started on port " + std::to_string(port));
    if (!isMaster) {
        size_t spacePos = masterHostAndPort.find(' ');
        if (spacePos != std::string::npos) {
            masterHost = masterHostAndPort.substr(0, spacePos);
            masterPort = std::stoi(masterHostAndPort.substr(spacePos + 1));
        }
        Logger::log("Server running as slave, master: " + masterHost + ":" + std::to_string(masterPort));
    }
}

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

void TcpServer::handleReplicaConnection() {
    int master_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (master_fd < 0) {
        Logger::error("Failed to create master socket");
        return;
    }

    Logger::log("Connecting to master at " + masterHost + ":" + std::to_string(masterPort));

    struct sockaddr_in master_addr;
    memset(&master_addr, 0, sizeof(master_addr));
    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(masterPort);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(masterHost.c_str(), std::to_string(masterPort).c_str(), &hints, &res);
    if (status != 0) {
        Logger::error("Failed to resolve master address: " + std::string(gai_strerror(status)));
        close(master_fd);
        return;
    }

    memcpy(&master_addr, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    if (connect(master_fd, (struct sockaddr *)&master_addr, sizeof(master_addr)) != 0) {
        Logger::error("Failed to connect to master: " + std::string(strerror(errno)));
        close(master_fd);
        return;
    }

    Logger::log("Connected to master " + masterHost + ":" + std::to_string(masterPort));

    std::string handshake = "*1\r\n$4\r\nPING\r\n";
    if (send(master_fd, handshake.c_str(), handshake.size(), 0) < 0) {
        Logger::error("Failed to send PING command to master");
        close(master_fd);
        return;
    }

    Logger::log("Sent PING command to master");

    // Keep the connection open
    // TODO: Implement replication logic here
}

void TcpServer::start() {
    if (!isMaster) {
        handleReplicaConnection();
    }

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