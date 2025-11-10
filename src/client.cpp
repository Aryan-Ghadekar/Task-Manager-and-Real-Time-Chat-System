// Enhanced JIRA-like Client with Error Handling
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include "../include/NetworkUtils.hpp"

std::string username;
int userId = -1;
bool authenticated = false;

void receiveMessages(int sock) {
    char buffer[1024];
    while (true) {
        try {
            int bytes = read(sock, buffer, sizeof(buffer)-1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::cout << "\n" << buffer << "\n> ";
                std::cout.flush();
            } else if (bytes == 0) {
                std::cout << "\n[SYSTEM] Connection closed by server\n";
                break;
            } else {
                std::cerr << "\n[ERROR] Read error: " << strerror(errno) << "\n";
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "\n[ERROR] Exception in receive: " << e.what() << "\n";
            break;
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
    std::cout << "/online                       - Show online users\n";
    std::cout << "/help                         - Show this help\n";
    std::cout << "/quit                         - Exit application\n";
    std::cout << "=====================================\n\n";
}

int main() {
    try {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
            return -1;
        }
        
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);
        
        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            close(sock);
            return -1;
        }

        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
            close(sock);
            return -1;
        }

        std::thread receiver(receiveMessages, sock);
        receiver.detach();

        std::cout << "Connected to JIRA-like Task Manager\n";
        showHelp();

        std::string input;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, input)) {
                break; // EOF or error
            }

            if (input == "/quit") break;
            if (input == "/help") {
                showHelp();
                continue;
            }
            
            if (input.empty()) continue;

            try {
                std::string message = NetworkUtils::formatMessage("CMD", input);
                ssize_t sent = send(sock, message.c_str(), message.size(), 0);
                if (sent < 0) {
                    std::cerr << "Failed to send message: " << strerror(errno) << std::endl;
                    break;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error sending command: " << e.what() << std::endl;
            }
        }
        
        close(sock);
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
