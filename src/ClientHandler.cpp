#include "ClientHandler.hpp"
#include "Logger.hpp"
#include "RDBDecoder.hpp"
#include <chrono>
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <arpa/inet.h>

DB_Config ClientHandler::config;
std::mutex ClientHandler::configMutex;

ClientHandler::ClientHandler() : parser() {}

void ClientHandler::handleClient(int clientSocket) {
    Logger::log("Handling client connection");
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
        } else if (command[0] == "KEYS") {
            handleKeys(command, clientSocket);
        } else {
            sendResponse(clientSocket, "-ERR unknown command\r\n");
        }
    }
    close(clientSocket);
    Logger::log("Client disconnected");
}

void ClientHandler::handleSet(const std::vector<std::string>& command, int clientSocket) {
    std::lock_guard<std::mutex> lock(configMutex);
    if (command.size() < 3) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'set' command\r\n");
        return;
    }

    Logger::log("SET command: key=" + command[1] + ", value=" + command[2]);
    DB_Entry entry;
    entry.value = command[2];
    entry.date = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    if (command.size() > 3 && command[3] == "px") {
        entry.expiry = entry.date + std::stoul(command[4]);
    } else {
        entry.expiry = 0; // No expiry
    }

    config.db[command[1]] = entry;
    sendResponse(clientSocket, "+OK\r\n");
}

void ClientHandler::handleGet(const std::vector<std::string>& command, int clientSocket) {
    std::lock_guard<std::mutex> lock(configMutex);
    Logger::log("GET command: key=" + command[1]);
    auto it = config.db.find(command[1]);
    if (it != config.db.end()) {
        uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        if (it->second.expiry == 0 || it->second.expiry > now) {
            std::string response = "$" + std::to_string(it->second.value.length()) + "\r\n" +
                                   it->second.value + "\r\n";
            sendResponse(clientSocket, response);
        } else {
            config.db.erase(it);
            sendResponse(clientSocket, "$-1\r\n");
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

    Logger::log("ECHO command: " + command[1]);
    std::string response = "$" + std::to_string(command[1].length()) + "\r\n" + command[1] + "\r\n";
    sendResponse(clientSocket, response);
}

void ClientHandler::handlePing(int clientSocket) {
    Logger::log("PING command");
    sendResponse(clientSocket, "+PONG\r\n");
}

void ClientHandler::handleConfig(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() < 3) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'config' command\r\n");
        return;
    }

    Logger::log("CONFIG command: " + command[1] + " " + command[2]);
    if (command[1] == "GET") {
        handleConfigGet(command[2], clientSocket);
    } else {
        sendResponse(clientSocket, "-ERR unknown subcommand for 'config' command\r\n");
    }
}

void ClientHandler::handleConfigGet(const std::string& parameter, int clientSocket) {
    std::string value;
    if (parameter == "dir") {
        value = config.dir;
    } else if (parameter == "dbfilename") {
        value = config.db_filename;
    } else {
        sendResponse(clientSocket, "-ERR unknown config parameter\r\n");
        return;
    }

    std::string response = "*2\r\n$" + std::to_string(parameter.length()) + "\r\n" + parameter + "\r\n" +
                           "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
    sendResponse(clientSocket, response);
}

void ClientHandler::handleKeys(const std::vector<std::string>& command, int clientSocket) {
    std::lock_guard<std::mutex> lock(configMutex);
    Logger::log("KEYS command");
    
    std::string response = "*" + std::to_string(config.db.size()) + "\r\n";
    for (const auto& pair : config.db) {
        response += "$" + std::to_string(pair.first.length()) + "\r\n" + pair.first + "\r\n";
    }
    sendResponse(clientSocket, response);
}

void ClientHandler::setInitialConfig(const std::string& initialDir, const std::string& initialDbfilename) {
    std::lock_guard<std::mutex> lock(configMutex);
    config.dir = initialDir;
    config.db_filename = initialDbfilename;
}

void ClientHandler::loadRdbFile() {
    std::lock_guard<std::mutex> lock(configMutex);
    RDBDecoder decoder(config);
    decoder.read_rdb();
    Logger::log("Loaded " + std::to_string(config.db.size()) + " keys from RDB file");
}

void ClientHandler::sendResponse(int clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.length(), 0);
}