// Enhanced JIRA-like Server
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include "../include/TaskManager.hpp"
#include "../include/ChatManager.hpp"
#include "../include/User.hpp"
#include "../include/NetworkUtils.hpp"

std::mutex serverMutex;
TaskManager taskManager;
ChatManager chatManager;
std::vector<ClientInfo> clients;
std::map<std::string, User> users;
int nextUserId = 1;

void initializeUsers() {
    users.insert({"admin", User(1, "admin", "admin@company.com", UserRole::ADMIN)});
    users.insert({"pm1", User(2, "pm1", "pm1@company.com", UserRole::PROJECT_MANAGER)});
    users.insert({"dev1", User(3, "dev1", "dev1@company.com", UserRole::DEVELOPER)});
    users.insert({"tester1", User(4, "tester1", "tester1@company.com", UserRole::TESTER)});
    nextUserId = 5;
}

ClientInfo* findClient(int socketId) {
    for (auto& client : clients) {
        if (client.socketId == socketId) return &client;
    }
    return nullptr;
}

void processCommand(int clientSock, const std::string& command) {
    std::lock_guard<std::mutex> lock(serverMutex);
    
    ClientInfo* client = findClient(clientSock);
    if (!client) return;
    
    std::vector<std::string> parts = NetworkUtils::splitString(command, ' ');
    if (parts.empty()) return;
    
    std::string cmd = parts[0];
    std::string response;
    
    if (cmd == "/login" && parts.size() >= 3) {
        std::string username = parts[1];
        if (users.find(username) != users.end()) {
            client->userId = users[username].getUserId();
            client->username = username;
            client->authenticated = true;
            users[username].setOnlineStatus(true);
            users[username].setSocketId(clientSock);
            
            response = "[SYSTEM] Welcome " + username + "! You are now logged in.";
            send(clientSock, response.c_str(), response.size(), 0);
            
            chatManager.sendSystemMessage(username + " joined the system");
            NetworkUtils::broadcastToAll(clients, "[SYSTEM] " + username + " is now online");
        } else {
            response = "[ERROR] Invalid credentials";
            send(clientSock, response.c_str(), response.size(), 0);
        }
    }
    else if (!client->authenticated) {
        response = "[ERROR] Please login first with /login <username> <password>";
        send(clientSock, response.c_str(), response.size(), 0);
        return;
    }
    else if (cmd == "/create" && parts.size() >= 2) {
        std::string taskData = command.substr(8);
        size_t pos = taskData.find(" | ");
        std::string title = (pos != std::string::npos) ? taskData.substr(0, pos) : taskData;
        std::string desc = (pos != std::string::npos) ? taskData.substr(pos + 3) : "";
        
        int taskId = taskManager.createTask(title, desc, client->userId, "PROJ");
        response = "[TASK] Created task PROJ-" + std::to_string(taskId) + ": " + title;
        
        NetworkUtils::broadcastToAll(clients, response);
        chatManager.sendTaskUpdate(client->userId, client->username, taskId, "Task created: " + title);
    }
    else if (cmd == "/assign" && parts.size() >= 3) {
        int taskId = std::stoi(parts[1]);
        int assigneeId = std::stoi(parts[2]);
        
        if (taskManager.assignTask(taskId, assigneeId, client->userId)) {
            response = "[TASK] Task " + std::to_string(taskId) + " assigned to user " + std::to_string(assigneeId);
            NetworkUtils::broadcastToAll(clients, response);
        } else {
            response = "[ERROR] Failed to assign task";
            send(clientSock, response.c_str(), response.size(), 0);
        }
    }
    else if (cmd == "/status" && parts.size() >= 3) {
        int taskId = std::stoi(parts[1]);
        std::string statusStr = parts[2];
        TaskStatus status = TaskStatus::TODO;
        
        if (statusStr == "PROGRESS") status = TaskStatus::IN_PROGRESS;
        else if (statusStr == "REVIEW") status = TaskStatus::IN_REVIEW;
        else if (statusStr == "DONE") status = TaskStatus::DONE;
        else if (statusStr == "BLOCKED") status = TaskStatus::BLOCKED;
        
        if (taskManager.updateTaskStatus(taskId, status, client->userId)) {
            response = "[TASK] Task " + std::to_string(taskId) + " status updated to " + statusStr;
            NetworkUtils::broadcastToAll(clients, response);
        }
    }
    else if (cmd == "/list") {
        auto tasks = taskManager.getAllTasks();
        response = "[TASKS] Current Tasks:\n";
        for (const auto& task : tasks) {
            response += task.toString() + "\n";
        }
        send(clientSock, response.c_str(), response.size(), 0);
    }
    else if (cmd == "/chat" && parts.size() >= 2) {
        std::string message = command.substr(6);
        chatManager.sendMessage(client->userId, client->username, message);
        response = "[" + client->username + "] " + message;
        NetworkUtils::broadcastToAll(clients, response);
    }
    else if (cmd == "/pm" && parts.size() >= 3) {
        int targetId = std::stoi(parts[1]);
        std::string message = command.substr(command.find(parts[2]));
        chatManager.sendPrivateMessage(client->userId, client->username, targetId, message);
        
        NetworkUtils::sendToUser(clients, targetId, "[PM from " + client->username + "] " + message);
        response = "[PM sent to user " + std::to_string(targetId) + "]";
        send(clientSock, response.c_str(), response.size(), 0);
    }
}

void handleClient(int clientSock) {
    ClientInfo client{clientSock, -1, "", false};
    
    {
        std::lock_guard<std::mutex> lock(serverMutex);
        clients.push_back(client);
    }
    
    std::string welcome = "[SYSTEM] Connected to JIRA-like Task Manager. Please login with /login <username> <password>";
    send(clientSock, welcome.c_str(), welcome.size(), 0);
    
    char buffer[1024];
    while (true) {
        int bytes = read(clientSock, buffer, sizeof(buffer)-1);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        std::string input(buffer);
        
        auto parsed = NetworkUtils::parseMessage(input);
        if (parsed.first == "CMD") {
            processCommand(clientSock, parsed.second);
        }
    }
    
    // Cleanup on disconnect
    {
        std::lock_guard<std::mutex> lock(serverMutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [clientSock](const ClientInfo& c) { return c.socketId == clientSock; }), clients.end());
        
        ClientInfo* disconnectedClient = findClient(clientSock);
        if (disconnectedClient && disconnectedClient->authenticated) {
            users[disconnectedClient->username].setOnlineStatus(false);
            NetworkUtils::broadcastToAll(clients, "[SYSTEM] " + disconnectedClient->username + " disconnected");
        }
    }
    
    close(clientSock);
}

int main() {
    initializeUsers();
    
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        return -1;
    }
    
    listen(serverSock, 10);
    std::cout << "JIRA-like Task Manager Server running on port 8080...\n";
    std::cout << "Available users: admin, pm1, dev1, tester1 (password: same as username)\n";
    
    while (true) {
        int clientSock = accept(serverSock, nullptr, nullptr);
        if (clientSock >= 0) {
            std::thread(handleClient, clientSock).detach();
        }
    }
    
    close(serverSock);
    return 0;
}
