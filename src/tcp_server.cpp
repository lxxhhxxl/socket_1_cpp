#include "tcp_server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

TcpServer::TcpServer(int port) : port_(port), server_fd_(-1), running_(false) {}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::initSocket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("socket failed");
        return false;
    }

    // 端口复用，避免重启时"Address already in use"
    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    addr.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return false;
    }

    if (listen(server_fd_, 5) < 0) {
        perror("listen failed");
        return false;
    }

    return true;
}

void TcpServer::handleClient(int client_fd) {
    char buffer[1024];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    
    getpeername(client_fd, (struct sockaddr*)&cliaddr, &len);
    std::cout << "[连接] 客户端 " << inet_ntoa(cliaddr.sin_addr) 
              << ":" << ntohs(cliaddr.sin_port) << std::endl;

    while (running_) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            std::cout << "[断开] 客户端离开" << std::endl;
            break;
        }

        buffer[n] = '\0';
        std::cout << "[收到] " << buffer << std::endl;

        // 回声
        send(client_fd, buffer, n, 0);
        std::cout << "[回传] " << buffer << std::endl;
    }

    close(client_fd);
}

bool TcpServer::start() {
    if (!initSocket()) return false;
    running_ = true;
    std::cout << "[服务器] 启动在端口 " << port_ << std::endl;
    return true;
}

void TcpServer::run() {
    while (running_) {
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&cliaddr, &len);
        if (client_fd < 0) {
            if (running_) perror("accept failed");
            continue;
        }

        // 多线程处理客户端
        std::thread(&TcpServer::handleClient, this, client_fd).detach();
    }
}

void TcpServer::stop() {
    running_ = false;
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
}