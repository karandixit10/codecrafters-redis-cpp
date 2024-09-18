#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <map>
#include "RedisParser.hpp"

class ClientHandler {
public:
    ClientHandler();
    void handleClient(int clientSocket);

private:
    std::map<std::string, std::string> keyValues;
    std::map<std::string, long> expTime;
    RedisParser parser;

    void handleSet(const std::vector<std::string>& command, int clientSocket);
    void handleGet(const std::vector<std::string>& command, int clientSocket);
    void handleEcho(const std::vector<std::string>& command, int clientSocket);
    void handlePing(int clientSocket);
    void sendResponse(int clientSocket, const std::string& response);
};

#endif // CLIENT_HANDLER_HPP