#include "ClientHandler.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

ClientHandler::ClientHandler() : parser() {}

void ClientHandler::handleClient(int clientSocket) {
    char recvBuffer[1024];
    long n;
    while ((n = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0) {
        recvBuffer[n] = '\0';
        std::vector<std::string> command = parser.parseCommand(recvBuffer);
        
        if (command.empty()) continue;

        if (command[0] == "SET") {
            handleSet(command, clientSocket);
        } else if (command[0] == "GET") {
            handleGet(command, clientSocket);
        } else if (command[0] == "ECHO") {
            handleEcho(command, clientSocket);
        } else if (command[0] == "PING") {
            handlePing(clientSocket);
        } else {
            sendResponse(clientSocket, "-ERR unknown command\r\n");
        }
    }
    close(clientSocket);
}

void ClientHandler::handleSet(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() < 3) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'set' command\r\n");
        return;
    }

    keyValues[command[1]] = command[2];
    if (command.size() > 3 && command[3] == "px") {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto value = now_ms.time_since_epoch();
        long current_time_ms = value.count();
        expTime[command[1]] = current_time_ms + std::stol(command[4]);
    } else {
        expTime[command[1]] = -1;
    }
    sendResponse(clientSocket, "+OK\r\n");
}

void ClientHandler::handleGet(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() != 2) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'get' command\r\n");
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long current_time_ms = value.count();

    if (keyValues.find(command[1]) != keyValues.end() && 
        (expTime[command[1]] == -1 || expTime[command[1]] > current_time_ms)) {
        std::string response = "$" + std::to_string(keyValues[command[1]].length()) + "\r\n" +
                               keyValues[command[1]] + "\r\n";
        sendResponse(clientSocket, response);
    } else {
        sendResponse(clientSocket, "$-1\r\n");
    }
}

void ClientHandler::handleEcho(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() < 2) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'echo' command\r\n");
        return;
    }

    std::string response = "$" + std::to_string(command[1].length()) + "\r\n" + command[1] + "\r\n";
    sendResponse(clientSocket, response);
}

void ClientHandler::handlePing(int clientSocket) {
    sendResponse(clientSocket, "+PONG\r\n");
}

void ClientHandler::sendResponse(int clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.length(), 0);
}