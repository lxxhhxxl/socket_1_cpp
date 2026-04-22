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

void TcpServer::setHandler(DataHandler handler) {
    handler_ = handler;
}

bool TcpServer::initSocket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("socket failed");
        return false;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
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

void TcpServer::defaultEchoHandler(int client_fd, const uint8_t* data, size_t len) {
    // 默认：原样回传
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t n = send(client_fd, data + total_sent, len - total_sent, 0);
        if (n < 0) {
            perror("send failed");
            return;
        }
        total_sent += n;
    }
    std::cout << "[回声] " << len << " 字节" << std::endl;
}

void TcpServer::handleClient(int client_fd) {
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    getpeername(client_fd, (struct sockaddr*)&cliaddr, &len);
    
    std::cout << "[连接] 客户端 " << inet_ntoa(cliaddr.sin_addr) 
              << ":" << ntohs(cliaddr.sin_port) << std::endl;

    // 协议选择：先接收4字节魔数/长度头
    uint8_t header[4];
    
    while (running_) {
        // 接收4字节头
        ssize_t n = recv(client_fd, header, sizeof(header), MSG_PEEK);
        if (n <= 0) break;
        
        // 判断模式：如果前4字节是文本长度头（小值），按字符串处理
        // 否则按二进制流传处理
        // 简化方案：用第一个字节作为模式标识
        
        // 更简单的方案：先接收4字节，判断是字符串还是文件
        n = recv(client_fd, header, sizeof(header), 0);
        if (n < 4) break;
        
        uint32_t first_val = ntohl(*reinterpret_cast<uint32_t*>(header));
        
        if (first_val <= 1024 * 1024) {
            // 假设是字符串模式：长度 <= 1MB
            uint32_t str_len = first_val;
            std::string msg(str_len, '\0');
            
            n = recv(client_fd, &msg[0], str_len, MSG_WAITALL);
            if (n != str_len) break;
            
            std::cout << "[收到字符串] " << str_len << " 字节: " << msg << std::endl;
            
            // 回传：长度头 + 数据
            uint32_t len_net = htonl(str_len);
            send(client_fd, &len_net, sizeof(len_net), 0);
            send(client_fd, msg.c_str(), str_len, 0);
        } else {
            // 文件模式：假设是文件大小
            // 这里简化处理，实际应该更严格的协议
            std::cout << "[收到文件] 大小: " << first_val << " 字节" << std::endl;
            
            // 接收文件数据并回传
            uint32_t file_size = first_val;
            uint8_t buffer[4096];
            uint32_t total = 0;
            
            // 回传文件大小头
            send(client_fd, header, sizeof(header), 0);
            
            while (total < file_size) {
                uint32_t to_read = std::min(static_cast<uint32_t>(sizeof(buffer)), file_size - total);
                n = recv(client_fd, buffer, to_read, 0);
                if (n <= 0) break;
                
                send(client_fd, buffer, n, 0);
                total += n;
            }
            std::cout << "[回传文件] " << total << " 字节" << std::endl;
        }
    }

    close(client_fd);
    std::cout << "[断开] 客户端离开" << std::endl;
}

bool TcpServer::start() {
    if (!initSocket()) return false;
    running_ = true;
    if (!handler_) handler_ = defaultEchoHandler;
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