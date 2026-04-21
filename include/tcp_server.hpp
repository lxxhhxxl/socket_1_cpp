#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP


#include <string>

class TcpServer{
    public:

    explicit TcpServer(int port = 8080);
    ~TcpServer();

    bool start();
    void stop();
    void run(); //阻塞运行

    private:

    int port_;
    int server_fd_;
    bool running_;

    bool initSocket();
    void handleClient(int client_fd);
};

#endif