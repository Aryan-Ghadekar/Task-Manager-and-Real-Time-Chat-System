#pragma once
#include "Chat.hpp"
#include "User.hpp"
#include <vector>
#include <map>
#include <mutex>

class ChatManager {
private:
    std::vector<Chat> messages;
    std::map<int, std::vector<int>> userConnections; // user -> socket IDs
    int nextMessageId;
    mutable std::mutex chatMutex;

public:
    ChatManager();
    
    // Message operations
    int sendMessage(int senderId, const std::string& senderName, const std::string& content, MessageType type = MessageType::GENERAL);
    int sendPrivateMessage(int senderId, const std::string& senderName, int targetUserId, const std::string& content);
    int sendTaskUpdate(int senderId, const std::string& senderName, int taskId, const std::string& update);
    int sendSystemMessage(const std::string& content);
    
    // Query operations
    std::vector<Chat> getAllMessages() const;
    std::vector<Chat> getRecentMessages(int limit = 50) const;
    std::vector<Chat> getMessagesByUser(int userId) const;
    std::vector<Chat> getTaskMessages(int taskId) const;
    std::vector<Chat> getPrivateMessages(int userId1, int userId2) const;
    
    // Connection management
    void addUserConnection(int userId, int socketId);
    void removeUserConnection(int userId, int socketId);
    std::vector<int> getUserSockets(int userId) const;
    
    void saveToFile() const;
    void loadFromFile();
};