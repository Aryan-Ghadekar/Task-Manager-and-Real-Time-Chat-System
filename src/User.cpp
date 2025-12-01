#include "../include/User.hpp"

User::User(int id, const std::string& name, const std::string& mail, UserRole r)
    : userId(id), username(name), email(mail), role(r), isOnline(false), socketId(-1) {}

std::string User::getRoleString() const {
    switch (role) {
        case UserRole::ADMIN: return "Admin";
        case UserRole::PROJECT_MANAGER: return "Project Manager";
        case UserRole::DEVELOPER: return "Developer";
        case UserRole::TESTER: return "Tester";
        default: return "Unknown";
    }
}

bool User::hasPermission(const std::string& action) const {
    if (role == UserRole::ADMIN) return true;
    
    if (action == "create_task" || action == "update_task") {
        return role == UserRole::PROJECT_MANAGER || role == UserRole::DEVELOPER;
    }
    
    // Only PM and Admin can assign tasks
    if (action == "assign_task") {
        return role == UserRole::PROJECT_MANAGER;
    }
    
    if (action == "delete_task") {
        return role == UserRole::PROJECT_MANAGER;
    }
    
    return true; 
}