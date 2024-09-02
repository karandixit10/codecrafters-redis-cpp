// TcpServer.hpp
#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include "ClientHandler.hpp"  // Include the ClientHandler header

class TcpServer
{
public:
    TcpServer(int port);
    void start();

private:
    int server_fd;
    struct sockaddr_in server_addr;
    int port;
    void acceptConnections();
};

#endif // TCPSERVER_HPP
