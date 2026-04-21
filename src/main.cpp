#include <iostream>
#include <string>
#include "tcp_server.hpp"
#include "tcp_client.hpp"

void runServer() {
    TcpServer server(8080);
    if (server.start()) {
        server.run();  // 阻塞
    }
}

void runClient() {
    TcpClient client("127.0.0.1", 8080);
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
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "用法: " << argv[0] << " [server|client]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        runServer();
    } else if (mode == "client") {
        runClient();
    } else {
        std::cout << "未知模式: " << mode << std::endl;
        return 1;
    }

    return 0;
}