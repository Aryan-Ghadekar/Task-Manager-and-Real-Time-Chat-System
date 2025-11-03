#pragma once
#include <string>
#include <vector>
#include <map>

struct ClientInfo {
    int socketId;
    int userId;
    std::string username;
    bool authenticated;
};

class NetworkUtils {
public:
    // Message formatting
    static std::string formatMessage(const std::string& type, const std::string& data);
    static std::pair<std::string, std::string> parseMessage(const std::string& message);
    
    // JSON-like serialization
    static std::string serializeTask(const class Task& task);
    static std::string serializeChat(const class Chat& chat);
    static std::string serializeUser(const class User& user);
    
    // Broadcasting
    static void broadcastToAll(const std::vector<ClientInfo>& clients, const std::string& message);
    static void broadcastToUsers(const std::vector<ClientInfo>& clients, const std::vector<int>& userIds, const std::string& message);
    static void sendToUser(const std::vector<ClientInfo>& clients, int userId, const std::string& message);
    
    // Authentication
    static bool authenticateUser(const std::string& username, const std::string& password);
    static std::string generateSessionToken(int userId);
    
    // Utility functions
    static std::string getCurrentTimestamp();
    static std::vector<std::string> splitString(const std::string& str, char delimiter);
    static std::string trim(const std::string& str);
};