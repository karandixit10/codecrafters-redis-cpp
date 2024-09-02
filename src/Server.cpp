#include <iostream>
#include "TcpServer.hpp"  // Include the TcpServer header

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cout << "Logs from your program will appear here!\n";

    int port = 6379;  // Default Redis port
    TcpServer server(port);
    server.start();

    return 0;
}
