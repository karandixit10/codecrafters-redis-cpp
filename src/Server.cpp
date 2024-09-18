#include "TcpServer.hpp"
#include <iostream>

int main() {
    try {
        TcpServer server(6379);  // Redis default port
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}