// ClientHandler.cpp
#include "ClientHandler.hpp"
#include "RedisParser.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <netdb.h>

#define BUFFER_SIZE 1024

// Initialize the static member variable
std::unordered_map<std::string, std::string> ClientHandler::key_value_store_;

ClientHandler::ClientHandler(int client_fd) : client_fd_(client_fd) {}

void ClientHandler::handle()
{
    char buff[BUFFER_SIZE] = "";

    while (true)
    {
        // Clear the buffer
        memset(buff, '\0', sizeof(buff));

        // Receive data from client
        ssize_t recv_bytes = recv(client_fd_, buff, sizeof(buff) - 1, 0);
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
            send(client_fd_, "-ERR empty command\r\n", 21, 0);
            continue;
        }

        // Process the command
        processCommand(argStr);
    }

    close(client_fd_);
}

void ClientHandler::processCommand(const std::vector<std::string>& argStr)
{
    if (argStr.empty())
    {
        send(client_fd_, "-ERR empty command\r\n", 21, 0);
        return;
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
    else if (command == "SET")
    {
        if (argStr.size() == 3)
        {
            key_value_store_[argStr[1]] = argStr[2];
            reply = "+OK\r\n";
        }
        else
        {
            reply = "-ERR wrong number of arguments for 'set' command\r\n";
        }
    }
    else if (command == "GET")
    {
        if (argStr.size() == 2)
        {
            auto it = key_value_store_.find(argStr[1]);
            if (it != key_value_store_.end())
            {
                reply = "$" + std::to_string(it->second.length()) + "\r\n" + it->second + "\r\n";
            }
            else
            {
                reply = "$-1\r\n"; // Null bulk string for non-existent keys
            }
        }
        else
        {
            reply = "-ERR wrong number of arguments for 'get' command\r\n";
        }
    }
    else
    {
        reply = "-ERR unknown command '" + command + "'\r\n";
    }

    // Send the reply to the client
    send(client_fd_, reply.c_str(), reply.size(), 0);
}