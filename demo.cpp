// Demo script to showcase the JIRA-like Task Manager features
#include <iostream>
#include <thread>
#include <chrono>
#include "include/TaskManager.hpp"
#include "include/ChatManager.hpp"
#include "include/User.hpp"

void demonstrateTaskManagement() {
    std::cout << "\n=== Task Management Demo ===\n";
    
    TaskManager taskManager;
    
    // Create users
    User admin(1, "admin", "admin@company.com", UserRole::ADMIN);
    User pm(2, "pm1", "pm1@company.com", UserRole::PROJECT_MANAGER);
    User dev(3, "dev1", "dev1@company.com", UserRole::DEVELOPER);
    
    std::cout << "Created users: " << admin.getUsername() << ", " << pm.getUsername() << ", " << dev.getUsername() << "\n";
    
    // Create tasks
    int task1 = taskManager.createTask("Implement user authentication", "Add login/logout functionality with session management", 2, "AUTH");
    int task2 = taskManager.createTask("Fix database connection", "Resolve connection timeout issues in production", 2, "DB");
    int task3 = taskManager.createTask("Update UI components", "Modernize the user interface with new design", 2, "UI");
    
    std::cout << "Created tasks: AUTH-" << task1 << ", DB-" << task2 << ", UI-" << task3 << "\n";
    
    // Assign tasks
    taskManager.assignTask(task1, 3, 2);  // Assign to developer
    taskManager.assignTask(task2, 3, 2);
    
    std::cout << "Assigned tasks to developer\n";
    
    // Update task status
    taskManager.updateTaskStatus(task1, TaskStatus::IN_PROGRESS, 3);
    taskManager.updateTaskPriority(task1, TaskPriority::HIGH, 2);
    
    std::cout << "Updated task status and priority\n";
    
    // Add comments
    taskManager.addTaskComment(task1, "Started working on authentication module", 3);
    taskManager.addTaskComment(task1, "Implemented basic login functionality", 3);
    
    std::cout << "Added comments to task\n";
    
    // Display all tasks
    auto tasks = taskManager.getAllTasks();
    std::cout << "\nCurrent Tasks:\n";
    for (const auto& task : tasks) {
        std::cout << "  " << task.toString() << "\n";
    }
    
    // Show task statistics
    auto statusCount = taskManager.getTaskStatusCount();
    std::cout << "\nTask Statistics:\n";
    std::cout << "  TODO: " << statusCount[TaskStatus::TODO] << "\n";
    std::cout << "  IN_PROGRESS: " << statusCount[TaskStatus::IN_PROGRESS] << "\n";
    std::cout << "  DONE: " << statusCount[TaskStatus::DONE] << "\n";
}

void demonstrateChatSystem() {
    std::cout << "\n=== Chat System Demo ===\n";
    
    ChatManager chatManager;
    
    // Send different types of messages
    chatManager.sendSystemMessage("System initialized successfully");
    chatManager.sendMessage(2, "pm1", "Welcome to the project team!", MessageType::GENERAL);
    chatManager.sendMessage(3, "dev1", "Thanks! Ready to start working.", MessageType::GENERAL);
    
    // Send task update
    chatManager.sendTaskUpdate(3, "dev1", 1, "Started working on authentication task");
    
    // Send private message
    chatManager.sendPrivateMessage(2, "pm1", 3, "Please prioritize the authentication task");
    
    std::cout << "Sent various message types\n";
    
    // Display recent messages
    auto messages = chatManager.getRecentMessages(10);
    std::cout << "\nRecent Messages:\n";
    for (const auto& message : messages) {
        std::cout << "  " << message.getFormattedMessage() << "\n";
    }
}

void demonstrateRealTimeFeatures() {
    std::cout << "\n=== Real-time Features Demo ===\n";
    std::cout << "In a real scenario, multiple clients would be connected simultaneously.\n";
    std::cout << "Features demonstrated:\n";
    std::cout << "  ✓ Multithreaded server handling multiple connections\n";
    std::cout << "  ✓ Real-time task updates broadcast to all clients\n";
    std::cout << "  ✓ Instant chat messaging\n";
    std::cout << "  ✓ Private messaging between users\n";
    std::cout << "  ✓ Task-related notifications\n";
    std::cout << "  ✓ User online/offline status tracking\n";
    std::cout << "  ✓ Role-based permissions\n";
    std::cout << "  ✓ Data persistence\n";
}

int main() {
    std::cout << "JIRA-like Task Manager and Real-Time Chat System Demo\n";
    std::cout << "====================================================\n";
    
    demonstrateTaskManagement();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    demonstrateChatSystem();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    demonstrateRealTimeFeatures();
    
    std::cout << "\n=== Demo Complete ===\n";
    std::cout << "To run the actual system:\n";
    std::cout << "1. Run 'make all' to build\n";
    std::cout << "2. Run './server' to start server\n";
    std::cout << "3. Run './client' to connect clients\n";
    std::cout << "4. Use /help command for available commands\n";
    
    return 0;
}