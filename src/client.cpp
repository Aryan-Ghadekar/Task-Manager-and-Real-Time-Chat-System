// src/client.cpp
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

void receiveMessages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes = read(sock, buffer, sizeof(buffer)-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << "\n[Update] " << buffer << "\n> ";
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::thread receiver(receiveMessages, sock);
    receiver.detach();

    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.rfind("/task", 0) == 0) {
            std::string msg = "TASK:" + input.substr(6);
            send(sock, msg.c_str(), msg.size(), 0);
        } else if (input.rfind("/chat", 0) == 0) {
            std::string msg = "CHAT:" + input.substr(6);
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }
    close(sock);
}
