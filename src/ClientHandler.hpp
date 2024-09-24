#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <string>
#include <mutex>
#include "RedisParser.hpp"
#include "DB.hpp"

class ClientHandler {
public:
    ClientHandler();
    void handleClient(int clientSocket);
    static void setInitialConfig(const std::string& initialDir, const std::string& initialDbfilename);
    static void loadRdbFile();

private:
    // Commands
    void handleSet(const std::vector<std::string>& command, int clientSocket);
    void handleGet(const std::vector<std::string>& command, int clientSocket);
    void handleEcho(const std::vector<std::string>& command, int clientSocket);
    void handlePing(int clientSocket);
    void handleConfig(const std::vector<std::string>& command, int clientSocket);
    void handleConfigGet(const std::string& parameter, int clientSocket);
    void handleKeys(const std::vector<std::string>& command, int clientSocket);

    // Utils
    void sendResponse(int clientSocket, const std::string& response);
    RedisParser parser;
    static DB_Config config;
    static std::mutex configMutex;
};

#endif // CLIENT_HANDLER_HPP