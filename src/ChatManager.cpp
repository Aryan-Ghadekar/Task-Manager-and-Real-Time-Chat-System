#include "../include/ChatManager.hpp"
#include "../include/Chat.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

ChatManager::ChatManager() : nextMessageId(1) {
    loadFromFile();
}

int ChatManager::sendMessage(int senderId, const std::string& senderName, const std::string& content, MessageType type) {
    std::lock_guard<std::mutex> lock(chatMutex);
    
    Chat newMessage(nextMessageId, senderId, senderName, content, type);
    messages.push_back(newMessage);
    
    int messageId = nextMessageId;
    nextMessageId++;
    
    saveToFile();
    return messageId;
}

int ChatManager::sendPrivateMessage(int senderId, const std::string& senderName, int targetUserId, const std::string& content) {
    std::lock_guard<std::mutex> lock(chatMutex);
    
    Chat newMessage(nextMessageId, senderId, senderName, content, MessageType::PRIVATE);
    newMessage.setTargetUser(targetUserId);
    messages.push_back(newMessage);
    
    int messageId = nextMessageId;
    nextMessageId++;
    
    saveToFile();
    return messageId;
}

int ChatManager::sendTaskUpdate(int senderId, const std::string& senderName, int taskId, const std::string& update) {
    std::lock_guard<std::mutex> lock(chatMutex);
    
    Chat newMessage(nextMessageId, senderId, senderName, update, MessageType::TASK_UPDATE);
    newMessage.setRelatedTask(taskId);
    messages.push_back(newMessage);
    
    int messageId = nextMessageId;
    nextMessageId++;
    
    saveToFile();
    return messageId;
}

int ChatManager::sendSystemMessage(const std::string& content) {
    std::lock_guard<std::mutex> lock(chatMutex);
    
    Chat newMessage(nextMessageId, 0, "System", content, MessageType::SYSTEM);
    messages.push_back(newMessage);
    
    int messageId = nextMessageId;
    nextMessageId++;
    
    saveToFile();
    return messageId;
}

std::vector<Chat> ChatManager::getAllMessages() const {
    std::lock_guard<std::mutex> lock(chatMutex);
    return messages;
}

std::vector<Chat> ChatManager::getRecentMessages(int limit) const {
    std::lock_guard<std::mutex> lock(chatMutex);
    std::vector<Chat> result;
    
    int start = std::max(0, static_cast<int>(messages.size()) - limit);
    for (size_t i = start; i < messages.size(); ++i) {
        result.push_back(messages[i]);
    }
    return result;
}

std::vector<Chat> ChatManager::getMessagesByUser(int userId) const {
    std::lock_guard<std::mutex> lock(chatMutex);
    std::vector<Chat> result;
    
    for (const auto& message : messages) {
        if (message.getSenderId() == userId) {
            result.push_back(message);
        }
    }
    return result;
}

std::vector<Chat> ChatManager::getTaskMessages(int taskId) const {
    std::lock_guard<std::mutex> lock(chatMutex);
    std::vector<Chat> result;
    
    for (const auto& message : messages) {
        if (message.getRelatedTaskId() == taskId) {
            result.push_back(message);
        }
    }
    return result;
}

std::vector<Chat> ChatManager::getPrivateMessages(int userId1, int userId2) const {
    std::lock_guard<std::mutex> lock(chatMutex);
    std::vector<Chat> result;
    
    for (const auto& message : messages) {
        if (message.getType() == MessageType::PRIVATE &&
            ((message.getSenderId() == userId1 && message.getTargetUserId() == userId2) ||
             (message.getSenderId() == userId2 && message.getTargetUserId() == userId1))) {
            result.push_back(message);
        }
    }
    return result;
}

void ChatManager::addUserConnection(int userId, int socketId) {
    std::lock_guard<std::mutex> lock(chatMutex);
    userConnections[userId].push_back(socketId);
}

void ChatManager::removeUserConnection(int userId, int socketId) {
    std::lock_guard<std::mutex> lock(chatMutex);
    auto& connections = userConnections[userId];
    connections.erase(std::remove(connections.begin(), connections.end(), socketId), connections.end());
}

std::vector<int> ChatManager::getUserSockets(int userId) const {
    std::lock_guard<std::mutex> lock(chatMutex);
    auto it = userConnections.find(userId);
    return (it != userConnections.end()) ? it->second : std::vector<int>();
}

void ChatManager::saveToFile() const {
    try {
        std::ofstream file("data/chatlog.txt", std::ios::app);
        if (file.is_open() && !messages.empty()) {
            const auto& lastMessage = messages.back();
            // Only save non-private messages in the chatlog.txt
            if (lastMessage.getType() != MessageType::PRIVATE) {
                file << lastMessage.getTimestamp() << " [" << lastMessage.getSenderName() << "] " 
                     << lastMessage.getContent() << "\n";
            }
            file.close();
        } else {
            std::cerr << "Failed to open chatlog.txt for writing" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving chat to file: " << e.what() << std::endl;
    }
}

void ChatManager::loadFromFile() {
    try {
        std::ifstream file("data/chatlog.txt");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                // Parse and load chat history if needed
                // For now, just count existing messages to set nextMessageId
                nextMessageId++;
            }
            file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading chat from file: " << e.what() << std::endl;
    }
}