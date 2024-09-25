#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>
#include "DB.hpp"

enum class CommandType {
    SET,
    GET,
    ECHO,
    PING,
    CONFIG,
    KEYS,
    INFO,
    UNKNOWN
};

class Commands {
public:
    static CommandType getCommandType(const std::string& command);
    static void handleSet(const std::vector<std::string>& command, int clientSocket, DB_Config& config);
    static void handleGet(const std::vector<std::string>& command, int clientSocket, DB_Config& config);
    static void handleEcho(const std::vector<std::string>& command, int clientSocket);
    static void handlePing(int clientSocket);
    static void handleConfig(const std::vector<std::string>& command, int clientSocket, DB_Config& config);
    static void handleKeys(const std::vector<std::string>& command, int clientSocket, DB_Config& config);
    static void handleInfo(const std::vector<std::string>& command, int clientSocket, DB_Config& config);

private:
    static void handleConfigGet(const std::string& parameter, int clientSocket, DB_Config& config);
    static void sendResponse(int clientSocket, const std::string& response);
};

#endif // COMMANDS_HPP
