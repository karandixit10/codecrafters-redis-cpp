// ClientHandler.hpp
#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <unordered_map>
#include <string>
#include <vector>

class ClientHandler {
public:
   
            ClientHandler   (int client_fd);                                // Constructor that accepts a client socket file descriptor

   
    void    handle          ();                                             // Method to handle client communication

private:
    void    processCommand (const std::vector<std::string>& argStr);        // Method to process commands from the client
    int     client_fd_;                                                     // Socket file descriptor for the client connection
    static  std::unordered_map<std::string, std::string> key_value_store_;  // Key-value store to manage data
};

#endif // CLIENTHANDLER_HPP