#include "TcpServer.hpp"
#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string dir;
    std::string dbfilename;

    int port = 6379;  // default port
    if (argc > 2 && std::string(argv[1]) == "--port") {
        port = std::stoi(argv[2]);
    }

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
        TcpServer server(port);
        Logger::log("Server started on port " + std::to_string(port));
        server.start();
    } catch (const std::exception& e) {
        Logger::error("Server error: " + std::string(e.what()));
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}