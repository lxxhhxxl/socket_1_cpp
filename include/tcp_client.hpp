#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <string>
#include <cstdint>

class TcpClient {
public:
    explicit TcpClient(const std::string& ip = "127.0.0.1", int port = 8080);
    ~TcpClient();

    bool connectToServer();
    void disconnect();
    
    // 字符串模式
    bool sendMessage(const std::string& msg);
    std::string receiveMessage();
    
    // 二进制文件模式
    bool sendFile(const std::string& filepath);
    bool receiveFile(const std::string& filepath, size_t expected_size = 0);

private:
    std::string server_ip_;
    int port_;
    int sockfd_;
    
    bool sendAll(const uint8_t* data, size_t len);
    bool recvAll(uint8_t* data, size_t len);
};

#endif // TCP_CLIENT_HPP