// TcpServer.cpp
#include "TcpServer.hpp"

TcpServer::TcpServer(int port) : port(port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        exit(1);
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt failed\n";
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Failed to bind to port " << port << "\n";
        exit(1);
    }
}

void TcpServer::start()
{
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "Listen failed\n";
        exit(1);
    }

    std::cout << "Waiting for a client to connect...\n";
    acceptConnections();
}

void TcpServer::acceptConnections()
{
    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0)
        {
            std::cerr << "Failed to accept client connection\n";
            continue;
        }
        std::cout << "Client connected\n";

        // Create a ClientHandler object and start handling the client in a new thread
        std::thread client_thread([client_fd]() {
            ClientHandler handler(client_fd);
            handler.handle(); });
        client_thread.detach();
    }
}