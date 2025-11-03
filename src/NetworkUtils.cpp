#include "../include/NetworkUtils.hpp"
#include "../include/Task.hpp"
#include "../include/Chat.hpp"
#include "../include/User.hpp"
#include <sstream>
#include <chrono>
#include <iomanip>
#include <sys/socket.h>

std::string NetworkUtils::formatMessage(const std::string& type, const std::string& data) {
    return type + ":" + data;
}

std::pair<std::string, std::string> NetworkUtils::parseMessage(const std::string& message) {
    size_t pos = message.find(':');
    if (pos != std::string::npos) {
        return {message.substr(0, pos), message.substr(pos + 1)};
    }
    return {"", message};
}

std::string NetworkUtils::serializeTask(const Task& task) {
    std::ostringstream oss;
    oss << "TASK|" << task.getTaskId() << "|" << task.getTitle() << "|" 
        << task.getStatusString() << "|" << task.getPriorityString();
    return oss.str();
}

std::string NetworkUtils::serializeChat(const Chat& chat) {
    std::ostringstream oss;
    oss << "CHAT|" << chat.getMessageId() << "|" << chat.getSenderName() << "|" 
        << chat.getContent() << "|" << chat.getTimestamp();
    return oss.str();
}

std::string NetworkUtils::serializeUser(const User& user) {
    std::ostringstream oss;
    oss << "USER|" << user.getUserId() << "|" << user.getUsername() << "|" 
        << user.getRoleString() << "|" << (user.getOnlineStatus() ? "Online" : "Offline");
    return oss.str();
}

void NetworkUtils::broadcastToAll(const std::vector<ClientInfo>& clients, const std::string& message) {
    for (const auto& client : clients) {
        if (client.authenticated) {
            send(client.socketId, message.c_str(), message.size(), 0);
        }
    }
}

void NetworkUtils::broadcastToUsers(const std::vector<ClientInfo>& clients, const std::vector<int>& userIds, const std::string& message) {
    for (const auto& client : clients) {
        if (client.authenticated && 
            std::find(userIds.begin(), userIds.end(), client.userId) != userIds.end()) {
            send(client.socketId, message.c_str(), message.size(), 0);
        }
    }
}

void NetworkUtils::sendToUser(const std::vector<ClientInfo>& clients, int userId, const std::string& message) {
    for (const auto& client : clients) {
        if (client.authenticated && client.userId == userId) {
            send(client.socketId, message.c_str(), message.size(), 0);
        }
    }
}

bool NetworkUtils::authenticateUser(const std::string& username, const std::string& password) {
    // Simple authentication - in real implementation, use proper hashing
    return username == password;
}

std::string NetworkUtils::generateSessionToken(int userId) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << "TOKEN_" << userId << "_" << time_t;
    return oss.str();
}

std::string NetworkUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<std::string> NetworkUtils::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

std::string NetworkUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}