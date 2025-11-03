#pragma once
#include <string>
#include <chrono>

enum class MessageType {
    GENERAL,
    TASK_UPDATE,
    SYSTEM,
    PRIVATE
};

class Chat {
private:
    int messageId;
    int senderId;
    std::string senderName;
    std::string content;
    MessageType type;
    std::chrono::system_clock::time_point timestamp;
    int targetUserId; // For private messages
    int relatedTaskId; // For task-related messages

public:
    Chat() : messageId(-1), senderId(-1), senderName(""), content(""), type(MessageType::GENERAL), targetUserId(-1), relatedTaskId(-1) {
        timestamp = std::chrono::system_clock::now();
    }
    Chat(int id, int sender, const std::string& name, const std::string& msg, MessageType t = MessageType::GENERAL);
    
    // Getters
    int getMessageId() const { return messageId; }
    int getSenderId() const { return senderId; }
    std::string getSenderName() const { return senderName; }
    std::string getContent() const { return content; }
    MessageType getType() const { return type; }
    int getTargetUserId() const { return targetUserId; }
    int getRelatedTaskId() const { return relatedTaskId; }
    
    // Setters
    void setTargetUser(int userId) { targetUserId = userId; }
    void setRelatedTask(int taskId) { relatedTaskId = taskId; }
    
    std::string getTypeString() const;
    std::string getFormattedMessage() const;
    std::string getTimestamp() const;
};