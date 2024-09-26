#include "ClientHandler.hpp"
#include "Logger.hpp"
#include "RDBDecoder.hpp"
#include "Commands.hpp"
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

        CommandType commandType = Commands::getCommandType(command[0]);

        switch (commandType) {
            case CommandType::SET:
                Commands::handleSet(command, clientSocket, config);
                break;
            case CommandType::GET:
                Commands::handleGet(command, clientSocket, config);
                break;
            case CommandType::ECHO:
                Commands::handleEcho(command, clientSocket);
                break;
            case CommandType::PING:
                Commands::handlePing(clientSocket);
                break;
            case CommandType::CONFIG:
                Commands::handleConfig(command, clientSocket, config);
                break;
            case CommandType::KEYS:
                Commands::handleKeys(command, clientSocket, config);
                break;
            case CommandType::INFO:
                Commands::handleInfo(command, clientSocket, config);
                break;
            default:
                sendResponse(clientSocket, "-ERR unknown command\r\n");
        }
    }
    close(clientSocket);
    Logger::log("Client disconnected");
}

void ClientHandler::setInitialConfig(const std::string& initialDir, const std::string& initialDbfilename, uint64_t& initialPort, bool& initialIsMaster, std::string& initialMasterHostAndPort, Role& initialRole) {
    std::lock_guard<std::mutex> lock(configMutex);
    config.dir = initialDir;
    config.db_filename = initialDbfilename;
    config.port = initialPort;
    config.isMaster = initialIsMaster;
    config.role = initialRole;
    config.masterHostAndPort = initialMasterHostAndPort;
    config.master_replid = "8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb";
    config.master_repl_offset = 0;
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