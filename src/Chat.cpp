#include "../include/Chat.hpp"
#include <sstream>
#include <iomanip>

Chat::Chat(int id, int sender, const std::string& name, const std::string& msg, MessageType t)
    : messageId(id), senderId(sender), senderName(name), content(msg), type(t), 
      targetUserId(-1), relatedTaskId(-1) {
    timestamp = std::chrono::system_clock::now();
}

std::string Chat::getTypeString() const {
    switch (type) {
        case MessageType::GENERAL: return "CHAT";
        case MessageType::TASK_UPDATE: return "TASK";
        case MessageType::SYSTEM: return "SYSTEM";
        case MessageType::PRIVATE: return "PRIVATE";
        default: return "UNKNOWN";
    }
}

std::string Chat::getFormattedMessage() const {
    std::ostringstream oss;
    
    if (type == MessageType::SYSTEM) {
        oss << "[SYSTEM] " << content;
    } else if (type == MessageType::PRIVATE) {
        oss << "[PM from " << senderName << "] " << content;
    } else if (type == MessageType::TASK_UPDATE) {
        oss << "[TASK UPDATE by " << senderName << "] " << content;
    } else {
        oss << "[" << senderName << "] " << content;
    }
    
    return oss.str();
}

std::string Chat::getTimestamp() const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}