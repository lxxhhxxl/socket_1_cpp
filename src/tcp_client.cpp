#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <string>

class TcpClient {
public:
    explicit TcpClient(const std::string& ip = "127.0.0.1", int port = 8080);
    ~TcpClient();

    bool connectToServer();
    void disconnect();
    bool sendMessage(const std::string& msg);
    std::string receiveMessage();

private:
    std::string server_ip_;
    int port_;
    int sockfd_;
};

#endif // TCP_CLIENT_HPP