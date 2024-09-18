#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

class TcpServer {
public:
    TcpServer(int port);
    void start();

private:
    int serverSocket;
    int port;

    void setupServerSocket();
};

#endif // TCP_SERVER_HPP