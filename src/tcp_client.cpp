#include "tcp_client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

TcpClient::TcpClient(const std::string& ip, int port)
    : server_ip_(ip), port_(port), sockfd_(-1) {}

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::connectToServer() {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        perror("socket failed");
        return false;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    inet_pton(AF_INET, server_ip_.c_str(), &servaddr.sin_addr);

    if (connect(sockfd_, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect failed");
        close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    std::cout << "[客户端] 连接服务器 " << server_ip_ << ":" << port_ << " 成功" << std::endl;
    return true;
}

void TcpClient::disconnect() {
    if (sockfd_ >= 0) {
        close(sockfd_);
        sockfd_ = -1;
    }
}

bool TcpClient::sendMessage(const std::string& msg) {
    if (sockfd_ < 0) return false;
    ssize_t n = send(sockfd_, msg.c_str(), msg.length(), 0);
    return n == static_cast<ssize_t>(msg.length());
}

std::string TcpClient::receiveMessage() {
    if (sockfd_ < 0) return "";
    
    char buffer[1024];
    ssize_t n = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return "";
    
    buffer[n] = '\0';
    return std::string(buffer);
}