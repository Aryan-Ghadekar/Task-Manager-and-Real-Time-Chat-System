#pragma once
#include <string>
#include <vector>

enum class UserRole {
    ADMIN,
    PROJECT_MANAGER,
    DEVELOPER,
    TESTER
};

class User {
private:
    int userId;
    std::string username;
    std::string email;
    UserRole role;
    bool isOnline;
    int socketId;

public:
    User() : userId(-1), username(""), email(""), role(UserRole::DEVELOPER), isOnline(false), socketId(-1) {}
    User(int id, const std::string& name, const std::string& mail, UserRole r);
    
    // Getters
    int getUserId() const { return userId; }
    std::string getUsername() const { return username; }
    std::string getEmail() const { return email; }
    UserRole getRole() const { return role; }
    bool getOnlineStatus() const { return isOnline; }
    int getSocketId() const { return socketId; }
    
    // Setters
    void setOnlineStatus(bool status) { isOnline = status; }
    void setSocketId(int sock) { socketId = sock; }
    void setRole(UserRole r) { role = r; }
    
    std::string getRoleString() const;
    bool hasPermission(const std::string& action) const;
};