#include "ClientHandler.hpp"
#include "RedisParser.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <netdb.h>

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
            std::cerr << "Client disconnected\n";
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
