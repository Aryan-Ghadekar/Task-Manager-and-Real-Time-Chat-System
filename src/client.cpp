// Enhanced JIRA-like Client
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include "../include/NetworkUtils.hpp"

std::string username;
int userId = -1;
bool authenticated = false;

void receiveMessages(int sock) {
    char buffer[1024];
    while (true) {
        int bytes = read(sock, buffer, sizeof(buffer)-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << "\n" << buffer << "\n> ";
            std::cout.flush();
        }
    }
}

void showHelp() {
    std::cout << "\n=== JIRA-like Task Manager Commands ===\n";
    std::cout << "/login <username> <password>  - Login to system\n";
    std::cout << "/create <title> | <desc>      - Create new task\n";
    std::cout << "/assign <taskId> <userId>     - Assign task to user\n";
    std::cout << "/status <taskId> <status>     - Update task status (TODO/PROGRESS/REVIEW/DONE/BLOCKED)\n";
    std::cout << "/priority <taskId> <priority> - Set priority (LOW/MEDIUM/HIGH/CRITICAL)\n";
    std::cout << "/comment <taskId> <comment>   - Add comment to task\n";
    std::cout << "/list [project]               - List all tasks or by project\n";
    std::cout << "/mytasks                      - Show my assigned tasks\n";
    std::cout << "/chat <message>               - Send chat message\n";
    std::cout << "/pm <userId> <message>        - Send private message\n";
    std::cout << "/help                         - Show this help\n";
    std::cout << "/quit                         - Exit application\n";
    std::cout << "=====================================\n\n";
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Failed to connect to server\n";
        return -1;
    }

    std::thread receiver(receiveMessages, sock);
    receiver.detach();

    std::cout << "Connected to JIRA-like Task Manager\n";
    showHelp();

    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "/quit") break;
        if (input == "/help") {
            showHelp();
            continue;
        }

        std::string message = NetworkUtils::formatMessage("CMD", input);
        send(sock, message.c_str(), message.size(), 0);
    }
    
    close(sock);
    return 0;
}
