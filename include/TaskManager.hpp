#pragma once
#include "Task.hpp"
#include "User.hpp"
#include <vector>
#include <map>
#include <mutex>

class TaskManager {
private:
    std::vector<Task> tasks;
    std::map<std::string, std::vector<int>> projectTasks; // project -> task IDs
    int nextTaskId;
    mutable std::mutex taskMutex;

public:
    TaskManager();
    
    // Task operations
    int createTask(const std::string& title, const std::string& description, int reporterId, const std::string& projectKey);
    bool updateTaskStatus(int taskId, TaskStatus status, int userId);
    bool updateTaskPriority(int taskId, TaskPriority priority, int userId);
    bool assignTask(int taskId, int assigneeId, int userId);
    bool addTaskComment(int taskId, const std::string& comment, int userId);
    
    // Query operations
    std::vector<Task> getAllTasks() const;
    std::vector<Task> getTasksByProject(const std::string& projectKey) const;
    std::vector<Task> getTasksByAssignee(int userId) const;
    std::vector<Task> getTasksByStatus(TaskStatus status) const;
    Task* getTaskById(int taskId);
    
    // Statistics
    std::map<TaskStatus, int> getTaskStatusCount() const;
    std::vector<Task> getRecentTasks(int limit = 10) const;
    
    void saveToFile() const;
    void loadFromFile();
};