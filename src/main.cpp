#include <iostream>
#include <string>
#include "tcp_server.hpp"
#include "tcp_client.hpp"

void printUsage(const char* prog) {
    std::cout << "用法: " << prog << " <mode> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "服务器模式:" << std::endl;
    std::cout << "  " << prog << " server [port]" << std::endl;
    std::cout << "  例: " << prog << " server 8080" << std::endl;
    std::cout << std::endl;
    std::cout << "客户端模式 - 字符串回声:" << std::endl;
    std::cout << "  " << prog << " client <server_ip> <port>" << std::endl;
    std::cout << "  例: " << prog << " client 127.0.0.1 8080" << std::endl;
    std::cout << std::endl;
    std::cout << "客户端模式 - 文件传输:" << std::endl;
    std::cout << "  " << prog << " sendfile <server_ip> <port> <filepath>" << std::endl;
    std::cout << "  例: " << prog << " sendfile 127.0.0.1 8080 test.jpg" << std::endl;
    std::cout << std::endl;
    std::cout << "客户端模式 - 接收文件:" << std::endl;
    std::cout << "  " << prog << " recvfile <server_ip> <port> <savepath>" << std::endl;
    std::cout << "  例: " << prog << " recvfile 127.0.0.1 8080 received.jpg" << std::endl;
}

void runServer(int port) {
    TcpServer server(port);
    if (server.start()) {
        server.run();
    }
}

void runClientString(const std::string& ip, int port) {
    TcpClient client(ip, port);
    if (!client.connectToServer()) return;

    std::string input;
    while (true) {
        std::cout << "\n输入消息 (quit退出): ";
        std::getline(std::cin, input);
        
        if (input == "quit") break;
        
        if (!client.sendMessage(input)) {
            std::cout << "发送失败" << std::endl;
            break;
        }

        std::string response = client.receiveMessage();
        std::cout << "服务器回传: " << response << std::endl;
    }
    
    client.disconnect();
}

void runClientSendFile(const std::string& ip, int port, const std::string& filepath) {
    TcpClient client(ip, port);
    if (!client.connectToServer()) return;
    
    if (!client.sendFile(filepath)) {
        std::cout << "发送文件失败" << std::endl;
        return;
    }
    
    // 接收回声文件
    std::string savepath = "echo_" + filepath.substr(filepath.find_last_of("/\\") + 1);
    if (client.receiveFile(savepath)) {
        std::cout << "文件回声已保存: " << savepath << std::endl;
    }
    
    client.disconnect();
}

void runClientRecvFile(const std::string& ip, int port, const std::string& savepath) {
    TcpClient client(ip, port);
    if (!client.connectToServer()) return;
    
    // 先发送一个请求（简化协议）
    client.sendMessage("REQUEST_FILE");
    
    // 接收文件
    if (client.receiveFile(savepath)) {
        std::cout << "文件已保存: " << savepath << std::endl;
    }
    
    client.disconnect();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "server") {
        int port = (argc > 2) ? std::stoi(argv[2]) : 8080;
        runServer(port);
    }
    else if (mode == "client") {
        if (argc < 4) {
            printUsage(argv[0]);
            return 1;
        }
        std::string ip = argv[2];
        int port = std::stoi(argv[3]);
        runClientString(ip, port);
    }
    else if (mode == "sendfile") {
        if (argc < 5) {
            printUsage(argv[0]);
            return 1;
        }
        std::string ip = argv[2];
        int port = std::stoi(argv[3]);
        std::string filepath = argv[4];
        runClientSendFile(ip, port, filepath);
    }
    else if (mode == "recvfile") {
        if (argc < 5) {
            printUsage(argv[0]);
            return 1;
        }
        std::string ip = argv[2];
        int port = std::stoi(argv[3]);
        std::string savepath = argv[4];
        runClientRecvFile(ip, port, savepath);
    }
    else {
        std::cout << "未知模式: " << mode << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}