// Enhanced JIRA-like Server with Error Handling
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cerrno>
#include <cstring>
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
    try {
        for (auto& client : clients) {
            if (client.socketId == socketId) return &client;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error finding client: " << e.what() << std::endl;
    }
    return nullptr;
}

std::vector<std::string> getOnlineUsers() {
    std::vector<std::string> onlineUsers;
    for (const auto& pair : users) {
        if (pair.second.getOnlineStatus()) {
            onlineUsers.push_back(pair.first + " (" + pair.second.getRoleString() + ")");
        }
    }
    return onlineUsers;
}

bool sendSafeMessage(int socket, const std::string& message) {
    try {
        ssize_t result = send(socket, message.c_str(), message.size(), MSG_NOSIGNAL);
        return result != -1;
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
        return false;
    }
}

void processCommand(int clientSock, const std::string& command) {
    try {
        std::lock_guard<std::mutex> lock(serverMutex);
        
        ClientInfo* client = findClient(clientSock);
        if (!client) return;
        
        std::vector<std::string> parts = NetworkUtils::splitString(command, ' ');
        if (parts.empty()) return;
        
        std::string cmd = parts[0];
        std::string response;
        
        if (cmd == "/login" && parts.size() >= 3) {
            std::string username = parts[1];
            std::string password = parts[2];
            
            if (users.find(username) != users.end() && NetworkUtils::authenticateUser(username, password)) {
                client->userId = users[username].getUserId();
                client->username = username;
                client->authenticated = true;
                users[username].setOnlineStatus(true);
                users[username].setSocketId(clientSock);
                
                response = "[SYSTEM] Welcome " + username + "! You are now logged in.";
                sendSafeMessage(clientSock, response);
                
                chatManager.sendSystemMessage(username + " joined the system");
                NetworkUtils::broadcastToAll(clients, "[SYSTEM] " + username + " is now online");
                
                auto onlineUsers = getOnlineUsers();
                std::string usersList = "[ONLINE] Users online: ";
                for (size_t i = 0; i < onlineUsers.size(); ++i) {
                    usersList += onlineUsers[i];
                    if (i < onlineUsers.size() - 1) usersList += ", ";
                }
                sendSafeMessage(clientSock, usersList);
            } else {
                response = "[ERROR] Invalid credentials";
                sendSafeMessage(clientSock, response);
            }
        }
        else if (!client->authenticated) {
            response = "[ERROR] Please login first with /login <username> <password>";
            sendSafeMessage(clientSock, response);
            return;
        }
        else if (cmd == "/create" && parts.size() >= 2) {
            std::string taskData = command.substr(8);
            size_t pos = taskData.find(" | ");
            std::string title = (pos != std::string::npos) ? taskData.substr(0, pos) : taskData;
            std::string desc = (pos != std::string::npos) ? taskData.substr(pos + 3) : "";
            
            // FIX: Improved deadline parsing - check both title and description
            int deadlineDays = 7; // Default
            std::string fullText = title + " " + desc;
            size_t deadlinePos = fullText.find("deadline:");
            if (deadlinePos != std::string::npos) {
                try {
                    std::string deadlineStr = fullText.substr(deadlinePos + 9);
                    size_t spacePos = deadlineStr.find(' ');
                    if (spacePos != std::string::npos) {
                        deadlineStr = deadlineStr.substr(0, spacePos);
                    }
                    deadlineDays = std::stoi(deadlineStr);
                    
                    // Remove deadline specification from description
                    if (desc.find("deadline:") != std::string::npos) {
                        size_t descDeadlinePos = desc.find("deadline:");
                        desc = desc.substr(0, descDeadlinePos);
                        // Trim trailing spaces
                        while (!desc.empty() && desc.back() == ' ') desc.pop_back();
                    }
                } catch (const std::exception& e) {
                    // Keep default if parsing fails
                }
            }
            
            int taskId = taskManager.createTask(title, desc, client->userId, "PROJ", deadlineDays);
            response = "[TASK] Created task PROJ-" + std::to_string(taskId) + ": " + title + " (Deadline: " + std::to_string(deadlineDays) + " days)";
            
            NetworkUtils::broadcastToAll(clients, response);
            chatManager.sendTaskUpdate(client->userId, client->username, taskId, "Task created: " + title);
        }
        else if (cmd == "/assign" && parts.size() >= 3) {
            // PERMISSION CHECK: Only PM and Admin can assign tasks
            std::string assignerUsername = client->username;
            User& assignerUser = users[assignerUsername];
            
            if (!assignerUser.hasPermission("assign_task")) {
                sendSafeMessage(clientSock, "[ERROR] Only Project Managers and Admins can assign tasks");
                return;
            }
            
            try {
                int taskId = std::stoi(parts[1]);
                int assigneeId = std::stoi(parts[2]);
                
                if (taskManager.assignTask(taskId, assigneeId, client->userId)) {
                    response = "[TASK] Task " + std::to_string(taskId) + " assigned to user " + std::to_string(assigneeId);
                    NetworkUtils::broadcastToAll(clients, response);
                } else {
                    sendSafeMessage(clientSock, "[ERROR] Failed to assign task");
                }
            } catch (const std::exception& e) {
                sendSafeMessage(clientSock, "[ERROR] Invalid task or user ID");
            }
        }
        else if (cmd == "/status" && parts.size() >= 3) {
            try {
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
            } catch (const std::exception& e) {
                sendSafeMessage(clientSock, "[ERROR] Invalid task ID");
            }
        }
        else if (cmd == "/list") {
            auto tasks = taskManager.getAllTasks();
            response = "[TASKS] Current Tasks:\n";
            if (tasks.empty()) {
                response += "No tasks found.\n";
            } else {
                for (const auto& task : tasks) {
                    response += task.toString() + "\n";
                }
            }
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/mytasks") {
            auto tasks = taskManager.getTasksByAssignee(client->userId);
            response = "[MY TASKS] Your assigned tasks:\n";
            if (tasks.empty()) {
                response += "No tasks assigned to you.\n";
            } else {
                for (const auto& task : tasks) {
                    response += task.toString() + "\n";
                }
            }
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/comment" && parts.size() >= 3) {
            try {
                int taskId = std::stoi(parts[1]);
                std::string comment = command.substr(command.find(parts[2]));
                
                if (taskManager.addTaskComment(taskId, comment, client->userId)) {
                    response = "[TASK] Comment added to task " + std::to_string(taskId);
                    NetworkUtils::broadcastToAll(clients, response);
                } else {
                    sendSafeMessage(clientSock, "[ERROR] Failed to add comment");
                }
            } catch (const std::exception& e) {
                sendSafeMessage(clientSock, "[ERROR] Invalid task ID");
            }
        }
        else if (cmd == "/priority" && parts.size() >= 3) {
            try {
                int taskId = std::stoi(parts[1]);
                std::string priorityStr = parts[2];
                TaskPriority priority = TaskPriority::MEDIUM;
                
                if (priorityStr == "LOW") priority = TaskPriority::LOW;
                else if (priorityStr == "MEDIUM") priority = TaskPriority::MEDIUM;
                else if (priorityStr == "HIGH") priority = TaskPriority::HIGH;
                else if (priorityStr == "CRITICAL") priority = TaskPriority::CRITICAL;
                else {
                    sendSafeMessage(clientSock, "[ERROR] Invalid priority. Use: LOW, MEDIUM, HIGH, CRITICAL");
                    return;
                }
                
                if (taskManager.updateTaskPriority(taskId, priority, client->userId)) {
                    response = "[TASK] Task " + std::to_string(taskId) + " priority updated to " + priorityStr;
                    NetworkUtils::broadcastToAll(clients, response);
                } else {
                    sendSafeMessage(clientSock, "[ERROR] Failed to update priority");
                }
            } catch (const std::exception& e) {
                sendSafeMessage(clientSock, "[ERROR] Invalid task ID");
            }
        }
        else if (cmd == "/chat" && parts.size() >= 2) {
            std::string message = command.substr(6);
            chatManager.sendMessage(client->userId, client->username, message);
            response = "[" + client->username + "] " + message;
            NetworkUtils::broadcastToAll(clients, response);
        }
        else if (cmd == "/pm" && parts.size() >= 3) {
            std::string target = parts[1];
            std::string message = command.substr(command.find(parts[2]));
            
            int targetId = -1;
            std::string targetUsername;
            
            // Try to find by username first
            if (users.find(target) != users.end()) {
                targetId = users[target].getUserId();
                targetUsername = target;
            } else {
                // Try to parse as user ID
                try {
                    int id = std::stoi(target);
                    for (const auto& user : users) {
                        if (user.second.getUserId() == id) {
                            targetId = id;
                            targetUsername = user.first;
                            break;
                        }
                    }
                } catch (const std::exception& e) {
                    // Not a valid number, continue with username search
                }
            }
            
            if (targetId == -1) {
                sendSafeMessage(clientSock, "[ERROR] User not found. Use username or user ID");
                return;
            }
            
            chatManager.sendPrivateMessage(client->userId, client->username, targetId, message);
            NetworkUtils::sendToUser(clients, targetId, "[PM from " + client->username + "] " + message);
            response = "[PM sent to " + targetUsername + "] " + message;
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/online") {
            auto onlineUsers = getOnlineUsers();
            response = "[ONLINE] Users currently online:\n";
            if (onlineUsers.empty()) {
                response += "No users online.\n";
            } else {
                for (const auto& user : onlineUsers) {
                    response += "- " + user + "\n";
                }
            }
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/dashboard") {
            // DASHBOARD: Generate comprehensive project dashboard
            response = taskManager.generateDashboard(users);
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/recommend") {
            // SMART ASSIGNMENT: Recommend best assignee
            int recommendedId = taskManager.recommendBestAssignee(users);
            if (recommendedId != -1) {
                std::string recommendedUser;
                for (const auto& user : users) {
                    if (user.second.getUserId() == recommendedId) {
                        recommendedUser = user.first;
                        break;
                    }
                }
                response = "[RECOMMEND] Best assignee: " + recommendedUser + " (ID: " + std::to_string(recommendedId) + ") - Current workload: " + std::to_string(taskManager.getActiveTaskCount(recommendedId)) + " tasks";
            } else {
                response = "[RECOMMEND] No suitable assignee found";
            }
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/overdue") {
            // Show overdue tasks
            auto overdueTasks = taskManager.getOverdueTasks();
            response = "[OVERDUE] Overdue tasks (" + std::to_string(overdueTasks.size()) + "):\n";
            if (overdueTasks.empty()) {
                response += "No overdue tasks.\n";
            } else {
                for (const auto& task : overdueTasks) {
                    response += "- " + task.toString() + "\n";
                }
            }
            sendSafeMessage(clientSock, response);
        }
        else if (cmd == "/help") {
            // FIX: Add help command handler
            response = "\n=== ENHANCED JIRA-like Task Manager Commands ===\n";
            response += "AUTHENTICATION:\n";
            response += "  /login <username> <password>  - Login to system\n\n";
            response += "TASK MANAGEMENT:\n";
            response += "  /create <title> | <desc> deadline:X - Create task (X=days)\n";
            response += "  /assign <taskId> <userId>     - Assign task (PM/Admin only)\n";
            response += "  /status <taskId> <status>     - Update status\n";
            response += "  /priority <taskId> <priority> - Set priority\n";
            response += "  /comment <taskId> <comment>   - Add comment\n";
            response += "  /list                         - List all tasks\n";
            response += "  /mytasks                      - Show my tasks\n\n";
            response += "SMART FEATURES:\n";
            response += "  /dashboard                    - Project dashboard\n";
            response += "  /recommend                    - Best assignee\n";
            response += "  /overdue                      - Overdue tasks\n\n";
            response += "COMMUNICATION:\n";
            response += "  /chat <message>               - Public message\n";
            response += "  /pm <user> <message>          - Private message\n";
            response += "  /online                       - Online users\n\n";
            response += "SYSTEM:\n";
            response += "  /help                         - This help\n";
            response += "  /quit                         - Exit\n";
            sendSafeMessage(clientSock, response);
        }
        else {
            sendSafeMessage(clientSock, "[ERROR] Unknown command. Type /help for available commands");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing command: " << e.what() << std::endl;
        sendSafeMessage(clientSock, "[ERROR] Internal server error");
    }
}

void handleClient(int clientSock) {
    try {
        ClientInfo client{clientSock, -1, "", false};
        
        {
            std::lock_guard<std::mutex> lock(serverMutex);
            clients.push_back(client);
        }
        
        std::string welcome = "[SYSTEM] Connected to JIRA-like Task Manager. Please login with /login <username> <password>";
        if (!sendSafeMessage(clientSock, welcome)) {
            close(clientSock);
            return;
        }
        
        char buffer[1024];
        while (true) {
            int bytes = read(clientSock, buffer, sizeof(buffer)-1);
            if (bytes <= 0) break;
            
            buffer[bytes] = '\0';
            std::string input(buffer);
            
            while (!input.empty() && (input.back() == '\n' || input.back() == '\r')) {
                input.pop_back();
            }
            
            if (!input.empty()) {
                auto parsed = NetworkUtils::parseMessage(input);
                if (parsed.first == "CMD" || parsed.first.empty()) {
                    processCommand(clientSock, parsed.second.empty() ? input : parsed.second);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }
    
    // Cleanup on disconnect
    try {
        std::lock_guard<std::mutex> lock(serverMutex);
        
        ClientInfo* disconnectedClient = nullptr;
        for (auto& client : clients) {
            if (client.socketId == clientSock) {
                disconnectedClient = &client;
                break;
            }
        }
        
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [clientSock](const ClientInfo& c) { return c.socketId == clientSock; }), clients.end());
        
        if (disconnectedClient && disconnectedClient->authenticated) {
            users[disconnectedClient->username].setOnlineStatus(false);
            NetworkUtils::broadcastToAll(clients, "[SYSTEM] " + disconnectedClient->username + " disconnected");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during cleanup: " << e.what() << std::endl;
    }
    
    close(clientSock);
}

int main() {
    try {
        initializeUsers();
        
        int serverSock = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSock < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return -1;
        }
        
        int opt = 1;
        if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            close(serverSock);
            return -1;
        }
        
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            close(serverSock);
            return -1;
        }
        
        if (listen(serverSock, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
            close(serverSock);
            return -1;
        }
        
        std::cout << "JIRA-like Task Manager Server running on port 8080...\n";
        std::cout << "Available users: admin, pm1, dev1, tester1 (password: same as username)\n";
        std::cout << "Server started successfully. Waiting for connections...\n";
        
        while (true) {
            int clientSock = accept(serverSock, nullptr, nullptr);
            if (clientSock >= 0) {
                std::cout << "New client connected: " << clientSock << std::endl;
                std::thread(handleClient, clientSock).detach();
            } else {
                std::cerr << "Accept failed" << std::endl;
            }
        }
        
        close(serverSock);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
