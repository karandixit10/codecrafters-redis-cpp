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
    RedisParser parser;
    static DB_Config config;
    static std::mutex configMutex;
    void sendResponse(int clientSocket, const std::string& response);
};

#endif // CLIENT_HANDLER_HPP