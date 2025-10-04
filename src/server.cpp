// src/server.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include "TaskManager.hpp"
#include "ChatManager.hpp"

std::mutex mtx;
TaskManager taskManager;
ChatManager chatManager;
std::vector<int> clients;

void handleClient(int clientSock) {
    char buffer[1024];
    while (true) {
        int bytes = read(clientSock, buffer, sizeof(buffer)-1);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';
        std::string input(buffer);

        std::lock_guard<std::mutex> lock(mtx);

        if (input.rfind("TASK:", 0) == 0) {
            taskManager.addTask(input.substr(5));
        } else if (input.rfind("CHAT:", 0) == 0) {
            chatManager.sendMessage("User", input.substr(5));
        }

        // Broadcast update to all clients
        for (int sock : clients) {
            send(sock, input.c_str(), input.size(), 0);
        }
    }
    close(clientSock);
}

int main() {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSock, 5);

    std::cout << "Server running on port 8080...\n";

    while (true) {
        int clientSock = accept(serverSock, nullptr, nullptr);
        clients.push_back(clientSock);
        std::thread(handleClient, clientSock).detach();
    }
    close(serverSock);
}
