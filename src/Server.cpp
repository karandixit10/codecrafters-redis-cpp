#include "TcpServer.hpp"
#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string dir;
    std::string dbfilename;

    // Parse command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            if (std::string(argv[i]) == "--dir") {
                dir = argv[i + 1];
            } else if (std::string(argv[i]) == "--dbfilename") {
                dbfilename = argv[i + 1];
            }
        }
    }

    // Set the initial configuration
    ClientHandler::setInitialConfig(dir, dbfilename);

    // Initialize the logger
    Logger::init("server.log");

    try {
        TcpServer server(6379);  // Redis default port
        Logger::log("Server started on port 6379");
        server.start();
    } catch (const std::exception& e) {
        Logger::error("Server error: " + std::string(e.what()));
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}