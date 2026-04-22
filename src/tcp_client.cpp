#include "tcp_client.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

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

// ========== 字符串模式 ==========

bool TcpClient::sendMessage(const std::string& msg) {
    if (sockfd_ < 0) return false;
    
    // 发送4字节长度头（网络字节序）+ 字符串数据
    uint32_t len = htonl(msg.length());
    if (!sendAll(reinterpret_cast<uint8_t*>(&len), sizeof(len))) return false;
    if (!sendAll(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length())) return false;
    
    return true;
}

std::string TcpClient::receiveMessage() {
    if (sockfd_ < 0) return "";
    
    // 接收4字节长度头
    uint32_t len_net;
    if (!recvAll(reinterpret_cast<uint8_t*>(&len_net), sizeof(len_net))) return "";
    uint32_t len = ntohl(len_net);
    
    if (len == 0 || len > 10 * 1024 * 1024) return ""; // 防止异常
    
    // 接收字符串数据
    std::string result(len, '\0');
    if (!recvAll(reinterpret_cast<uint8_t*>(&result[0]), len)) return "";
    
    return result;
}

// ========== 二进制文件模式 ==========

bool TcpClient::sendFile(const std::string& filepath) {
    if (sockfd_ < 0) return false;
    
    // 打开文件
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filepath << std::endl;
        return false;
    }
    
    // 获取文件大小
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::cout << "[发送文件] " << filepath << " (" << file_size << " 字节)" << std::endl;
    
    // 发送8字节文件大小头（网络字节序）
    uint64_t size_net = htonl(file_size);  // 注意：Linux需要自定义或用be64toh
    // 简化：用4字节，限制4GB文件
    uint32_t size_32 = static_cast<uint32_t>(file_size);
    uint32_t size_net_32 = htonl(size_32);
    if (!sendAll(reinterpret_cast<uint8_t*>(&size_net_32), sizeof(size_net_32))) return false;
    
    // 发送文件内容
    uint8_t buffer[4096];
    std::streamsize total_sent = 0;
    
    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        std::streamsize n = file.gcount();
        if (n > 0) {
            if (!sendAll(buffer, n)) return false;
            total_sent += n;
        }
    }
    
    std::cout << "[发送完成] " << total_sent << " 字节" << std::endl;
    return true;
}

bool TcpClient::receiveFile(const std::string& filepath, size_t expected_size) {
    if (sockfd_ < 0) return false;
    
    // 接收4字节文件大小头
    uint32_t size_net;
    if (!recvAll(reinterpret_cast<uint8_t*>(&size_net), sizeof(size_net))) return false;
    uint32_t file_size = ntohl(size_net);
    
    std::cout << "[接收文件] 预期 " << file_size << " 字节，保存到 " << filepath << std::endl;
    
    // 创建文件
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filepath << std::endl;
        return false;
    }
    
    // 接收文件内容
    uint8_t buffer[4096];
    uint32_t total_received = 0;
    
    while (total_received < file_size) {
        uint32_t to_read = std::min(static_cast<uint32_t>(sizeof(buffer)), file_size - total_received);
        
        ssize_t n = recv(sockfd_, buffer, to_read, 0);
        if (n <= 0) {
            std::cerr << "接收中断: " << n << std::endl;
            return false;
        }
        
        file.write(reinterpret_cast<char*>(buffer), n);
        total_received += n;
    }
    
    std::cout << "[接收完成] " << total_received << " 字节" << std::endl;
    return true;
}

// ========== 底层辅助函数 ==========

bool TcpClient::sendAll(const uint8_t* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(sockfd_, data + total, len - total, 0);
        if (n < 0) {
            perror("send failed");
            return false;
        }
        total += n;
    }
    return true;
}

bool TcpClient::recvAll(uint8_t* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(sockfd_, data + total, len - total, 0);
        if (n <= 0) {
            if (n < 0) perror("recv failed");
            return false;
        }
        total += n;
    }
    return true;
}