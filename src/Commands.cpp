#include "Commands.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <chrono>
#include <sys/socket.h>
#include <iostream>
#include <string>
#include "base64.h" 

std::vector<int> Commands::replica_fds;

CommandType Commands::getCommandType(const std::string& command) {
    if (command == "SET") return CommandType::SET;
    if (command == "GET") return CommandType::GET;
    if (command == "ECHO") return CommandType::ECHO;
    if (command == "PING") return CommandType::PING;
    if (command == "CONFIG") return CommandType::CONFIG;
    if (command == "KEYS") return CommandType::KEYS;
    if (command == "INFO") return CommandType::INFO;
    if (command == "REPLCONF") return CommandType::REPLCONF;
    if (command == "PSYNC") return CommandType::PSYNC;
    return CommandType::UNKNOWN;
}

void Commands::handleSet(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
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

    // Propagate the command to replicas
    std::string propagateCmd = "*3\r\n$3\r\nSET\r\n$" + std::to_string(command[1].length()) + "\r\n" + command[1] + "\r\n$" + std::to_string(command[2].length()) + "\r\n" + command[2] + "\r\n";
    for (int replicaFd : replica_fds) {
        sendResponse(replicaFd, propagateCmd);
    }
}

void Commands::handleGet(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
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

void Commands::handleEcho(const std::vector<std::string>& command, int clientSocket) {
    if (command.size() < 2) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'echo' command\r\n");
        return;
    }

    Logger::log("ECHO command: " + command[1]);
    std::string response = "$" + std::to_string(command[1].length()) + "\r\n" + command[1] + "\r\n";
    sendResponse(clientSocket, response);
}

void Commands::handlePing(int clientSocket) {
    Logger::log("PING command");
    sendResponse(clientSocket, "+PONG\r\n");
}

void Commands::handleConfig(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
    if (command.size() < 3) {
        sendResponse(clientSocket, "-ERR wrong number of arguments for 'config' command\r\n");
        return;
    }

    Logger::log("CONFIG command: " + command[1] + " " + command[2]);
    if (command[1] == "GET") {
        handleConfigGet(command[2], clientSocket, config);
    } else {
        sendResponse(clientSocket, "-ERR unknown subcommand for 'config' command\r\n");
    }
}

void Commands::handleKeys(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
    Logger::log("KEYS command");
    
    std::string response = "*" + std::to_string(config.db.size()) + "\r\n";
    for (const auto& pair : config.db) {
        response += "$" + std::to_string(pair.first.length()) + "\r\n" + pair.first + "\r\n";
    }
    sendResponse(clientSocket, response);
}

void Commands::handleInfo(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
    Logger::log("INFO command");
    std::string response;

    if (command.size() > 1 && command[1] == "replication") {
        response = "# Replication\r\n";
        response += (config.role == Role::Master) ? "role:master\r\n" : "role:slave\r\n";
        response += "master_replid:" + config.master_replid + "\r\n";
        response += "master_repl_offset:" + std::to_string(config.master_repl_offset) + "\r\n";
    } else {
        // Handle general INFO command or other sections if needed
        response = "# Server\r\nredis_version:1.0.0\r\n# Replication\r\n";
        response += config.role == Role::Master ? "role:master\r\n" : "role:slave\r\n";
        response += "master_replid:" + config.master_replid + "\r\n";
        response += "master_repl_offset:" + std::to_string(config.master_repl_offset) + "\r\n";
    }

    // Encode the response as a Bulk string
    std::string bulkResponse = "$" + std::to_string(response.length()) + "\r\n" + response + "\r\n";
    sendResponse(clientSocket, bulkResponse);
}

void Commands::handleConfigGet(const std::string& parameter, int clientSocket, DB_Config& config) {
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

void Commands::handleReplconf(const std::vector<std::string>& command, int clientSocket) {
    Logger::log("REPLCONF command: " + command[1] + " " + command[2]);
    sendResponse(clientSocket, "+OK\r\n");
}

void Commands::handlePsync(const std::vector<std::string>& command, int clientSocket, DB_Config& config) {
    Logger::log("PSYNC command: " + command[1] + " " + command[2]);
    std::string response = "+FULLRESYNC " + config.master_replid + " " + std::to_string(config.master_repl_offset) + "\r\n";
    sendResponse(clientSocket, response);
    replica_fds.push_back(clientSocket);

    // Send the empty RDB file
    sendRdbFile(clientSocket);
}

void Commands::sendRdbFile(int clientSocket) {
    // Empty RDB file contents in base64
    const std::string emptyRdbBase64 = "UkVESVMwMDEx+glyZWRpcy12ZXIFNy4yLjD6CnJlZGlzLWJpdHPAQPoFY3RpbWXCbQi8ZfoIdXNlZC1tZW3CsMQQAPoIYW9mLWJhc2XAAP/wbjv+wP9aog==";
    
    // Decode base64 to binary
    std::string emptyRdbBinary = base64_decode(emptyRdbBase64);
    
    // Send the RDB file size
    std::string sizePrefix = "$" + std::to_string(emptyRdbBinary.size()) + "\r\n";
    sendResponse(clientSocket, sizePrefix);
    
    // Send the RDB file contents
    sendResponse(clientSocket, emptyRdbBinary);
}

void Commands::sendResponse(int clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.length(), 0);
}
