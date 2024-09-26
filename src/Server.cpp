#include "TcpServer.hpp"
#include "ClientHandler.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string dir;
    std::string dbfilename;
    bool isMaster = true;
    Role role = Role::Master;
    uint64_t port = 6379;  // default port
    std::string masterHostAndPort;
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (std::string(argv[i]) == "--dir" && i + 1 < argc) {
            dir = argv[i + 1];
            ++i;
        } else if (std::string(argv[i]) == "--dbfilename" && i + 1 < argc) {
            dbfilename = argv[i + 1];
            ++i;
        } else if (std::string(argv[i]) == "--port" && i + 1 < argc) {
            port = std::stoull(argv[i + 1]);
            ++i;
        } else if (std::string(argv[i]) == "--replicaof" && i + 1 < argc) {
            masterHostAndPort = argv[i + 1];
            isMaster = false;
            role = Role::Slave;
            i += 2;
        }
    }

    // Set the initial configuration
    ClientHandler::setInitialConfig(dir, dbfilename, port, isMaster, masterHostAndPort, role);

    // Initialize the logger
    //Logger::init("server.log");

    try {
        TcpServer server(port, isMaster, masterHostAndPort);
        Logger::log("Server started on port " + std::to_string(port));
        if (!isMaster) {
            Logger::log("Server running as slave, master: " + masterHostAndPort);
        }
        server.start();
    } catch (const std::exception& e) {
        Logger::error("Server error: " + std::string(e.what()));
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}