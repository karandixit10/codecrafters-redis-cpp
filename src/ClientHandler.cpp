#include "ClientHandler.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <mutex>
#include <algorithm>

ClientHandler::ClientHandler() : parser() {}

void ClientHandler::handleClient(int clientSocket) {
    char recvBuffer[1024];
    long n;
    while ((n = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0) {
        recvBuffer[n] = '\0';
        std::vector<std::string> command = parser.parseCommand(recvBuffer);
        
        if (command.empty()) continue;

        // Transform the command to uppercase
        std::transform(command[0].begin(), command[0].end(), command[0].begin(), ::toupper);

        if (command[0] == "SET") {
            handleSet(command, clientSocket);
        } else if (command[0] == "GET") {
            handleGet(command, clientSocket);
        } else if (command[0] == "ECHO") {
            handleEcho(command, clientSocket);
        } else if (command[0] == "PING") {
            handlePing(clientSocket);
        } else if (command[0] == "CONFIG") {
            handleConfig(command, clientSocket);
        } else {
            sendResponse(clientSocket, "-ERR unknown command\r\n");
        }
    }
    close(clientSocket);
}

void ClientHandler::handleSet(const std::vector<std::string>& command, int clientSocket) {
    std::lock_guard<std::mutex> lock(mapMutex);
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
    std::lock_guard<std::mutex> lock(mapMutex);
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    long current_time_ms = value.count();

    if (keyValues.find(command[1]) != keyValues.end()) {
        long exp_time = expTime[command[1]];
        if (exp_time == -1 || exp_time > current_time_ms) {
            std::string response = "$" + std::to_string(keyValues[command[1]].length()) + "\r\n" +
                                   keyValues[command[1]] + "\r\n";
            sendResponse(clientSocket, response);
        } else {
            sendResponse(clientSocket, "$-1\r\n");
            keyValues.erase(command[1]);
            expTime.erase(command[1]);
        }
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

void ClientHandler::handleConfig(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() < 3) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'config' command\r\n");
        return;
    }

    if (command[1] == "GET") {
        handleConfigGet(command[2], clientSocket);
    } else {
        sendResponse(clientSocket, "-ERR unknown subcommand for 'config' command\r\n");
    }
}

void ClientHandler::handleConfigGet(const std::string& parameter, int clientSocket) {
    std::string value;
    if (parameter == "dir") {
        value = dir;
    } else if (parameter == "dbfilename") {
        value = dbfilename;
    } else {
        sendResponse(clientSocket, "-ERR unknown config parameter\r\n");
        return;
    }

    std::string response = "*2\r\n$" + std::to_string(parameter.length()) + "\r\n" + parameter + "\r\n" +
                           "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
    sendResponse(clientSocket, response);
}

void ClientHandler::setInitialConfig(const std::string& initialDir, const std::string& initialDbfilename) {
    dir = initialDir;
    dbfilename = initialDbfilename;
}

std::map<std::string, std::string> ClientHandler::keyValues;
std::map<std::string, long> ClientHandler::expTime;
std::mutex ClientHandler::mapMutex;
// Initialize static members without default values
std::string ClientHandler::dir = "/tmp/redis-data";
std::string ClientHandler::dbfilename = "dump.rdb";