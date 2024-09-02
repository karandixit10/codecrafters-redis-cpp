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
#include <algorithm> // for std::transform
#include "RedisParser.hpp"

#define BUFFER_SIZE 1024

void handle_client(int client_fd)
{
    char buff[BUFFER_SIZE] = "";

    while (true)
    {
        // Clear the buffer
        memset(buff, '\0', sizeof(buff));

        // Receive data from client
        ssize_t recv_bytes = recv(client_fd, buff, sizeof(buff) - 1, 0);
        if (recv_bytes <= 0)
        {
            std::cerr << "Client disconnected or error occurred\n";
            break;
        }

        std::string recv_str(buff, recv_bytes);
        RedisParser parser(recv_str);
        std::vector<std::string> argStr = parser.parseCommand();

        if (argStr.empty())
        {
            send(client_fd, "-ERR empty command\r\n", 21, 0);
            continue;
        }

        // Convert command to uppercase
        std::string command = argStr[0];
        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

        std::string reply;

        if (command == "PING")
        {
            reply = "+PONG\r\n";
        }
        else if (command == "ECHO")
        {
            if (argStr.size() > 1)
            {
                reply = "$" + std::to_string(argStr[1].length()) + "\r\n" + argStr[1] + "\r\n";
            }
            else
            {
                reply = "-ERR wrong number of arguments for 'echo' command\r\n";
            }
        }
        else
        {
            reply = "-ERR unknown command '" + command + "'\r\n";
        }

        // Send the reply to the client
        send(client_fd, reply.c_str(), reply.size(), 0);
    }

    close(client_fd);
}

int main(int argc, char **argv)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Failed to bind to port 6379\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "listen failed\n";
        return 1;
    }

    std::cout << "Waiting for a client to connect...\n";

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

        // Create a new thread to handle the client
        std::thread client_thread(handle_client, client_fd);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}
