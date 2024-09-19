#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "RedisParser.hpp"

class ClientHandler {
public:
    ClientHandler();
    void handleClient(int clientSocket);
    static void setInitialConfig(const std::string& initialDir, const std::string& initialDbfilename);

private:
    void handleSet(const std::vector<std::string>& command, int clientSocket);
    void handleGet(const std::vector<std::string>& command, int clientSocket);
    void handleEcho(const std::vector<std::string>& command, int clientSocket);
    void handlePing(int clientSocket);
    void handleConfig(const std::vector<std::string>& command, int clientSocket);
    void sendResponse(int clientSocket, const std::string& response);
    void handleConfigGet(const std::string& parameter, int clientSocket);

    RedisParser parser;
    static std::map<std::string, std::string> keyValues;
    static std::map<std::string, long> expTime;
    static std::mutex mapMutex;
    static std::string dir;
    static std::string dbfilename;
};

#endif // CLIENT_HANDLER_HPP