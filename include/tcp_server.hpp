#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <functional>
#include <cstdint>

// 回调函数类型：处理接收到的数据
using DataHandler = std::function<void(int client_fd, const uint8_t* data, size_t len)>;

class TcpServer {
public:
    explicit TcpServer(int port = 8080);
    ~TcpServer();

    bool start();
    void stop();
    void run();
    
    // 设置数据处理器（如果不设置，默认回声）
    void setHandler(DataHandler handler);

private:
    int port_;
    int server_fd_;
    bool running_;
    DataHandler handler_;

    bool initSocket();
    void handleClient(int client_fd);
    
    // 默认回声处理器
    static void defaultEchoHandler(int client_fd, const uint8_t* data, size_t len);
};

#endif // TCP_SERVER_HPP