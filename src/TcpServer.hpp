#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <cstdint>

class TcpServer {
public:
    TcpServer(int port, bool isMaster, const std::string& masterHostAndPort);
    void start();

private:
    int serverSocket;
    int port;
    bool isMaster;
    std::string masterHost;
    uint64_t masterPort;

    void setupServerSocket();
    void handleReplicaConnection();
};

#endif // TCP_SERVER_HPP