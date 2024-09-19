#include "TcpServer.hpp"
#include "ClientHandler.hpp"
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

    try {
        TcpServer server(6379);  // Redis default port
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}