// TcpServer.hpp
#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <netinet/in.h>
#include <thread>
#include <iostream>
#include "ClientHandler.hpp"

class TcpServer {
public:
            TcpServer   (int port);
    void    start       ();                          // Method to start the server

private:
    
    void acceptConnections  ();                      // Method to accept client connections

    int                     port;                    // Port number for the server
    int                     server_fd;               // File descriptor for the server socket
    struct sockaddr_in      server_addr;             // Server address structure
    void handleClient       (int client_fd);         // Method to handle a client connection
};

#endif // TCPSERVER_HPP