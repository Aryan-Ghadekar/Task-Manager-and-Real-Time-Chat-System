#include "../include/TaskManager.hpp"
#include "../include/Task.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

TaskManager::TaskManager() : nextTaskId(1) {
    loadFromFile();
}

int TaskManager::createTask(const std::string& title, const std::string& description, int reporterId, const std::string& projectKey) {
    try {
        std::lock_guard<std::mutex> lock(taskMutex);
        
        if (title.empty()) {
            throw std::invalid_argument("Task title cannot be empty");
        }
        
        Task newTask(nextTaskId, title, description, reporterId, projectKey);
        tasks.push_back(newTask);
        projectTasks[projectKey].push_back(nextTaskId);
        
        int taskId = nextTaskId;
        nextTaskId++;
        
        saveToFile();
        return taskId;
    } catch (const std::exception& e) {
        std::cerr << "Error creating task: " << e.what() << std::endl;
        return -1;
    }
}

bool TaskManager::updateTaskStatus(int taskId, TaskStatus status, int userId) {
    try {
        std::lock_guard<std::mutex> lock(taskMutex);
        
        Task* task = getTaskById(taskId);
        if (task) {
            task->setStatus(status);
            saveToFile();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error updating task status: " << e.what() << std::endl;
        return false;
    }
}

bool TaskManager::updateTaskPriority(int taskId, TaskPriority priority, int userId) {
    std::lock_guard<std::mutex> lock(taskMutex);
    
    Task* task = getTaskById(taskId);
    if (task) {
        task->setPriority(priority);
        saveToFile();
        return true;
    }
    return false;
}

bool TaskManager::assignTask(int taskId, int assigneeId, int userId) {
    try {
        std::lock_guard<std::mutex> lock(taskMutex);
        
        Task* task = getTaskById(taskId);
        if (task) {
            task->setAssignee(assigneeId);
            saveToFile();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error assigning task: " << e.what() << std::endl;
        return false;
    }
}

bool TaskManager::addTaskComment(int taskId, const std::string& comment, int userId) {
    std::lock_guard<std::mutex> lock(taskMutex);
    
    Task* task = getTaskById(taskId);
    if (task) {
        task->addComment(comment);
        saveToFile();
        return true;
    }
    return false;
}

std::vector<Task> TaskManager::getAllTasks() const {
    std::lock_guard<std::mutex> lock(taskMutex);
    return tasks;
}

std::vector<Task> TaskManager::getTasksByProject(const std::string& projectKey) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        if (task.getProjectKey() == projectKey) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskManager::getTasksByAssignee(int userId) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        if (task.getAssigneeId() == userId) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskManager::getTasksByStatus(TaskStatus status) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        if (task.getStatus() == status) {
            result.push_back(task);
        }
    }
    return result;
}

Task* TaskManager::getTaskById(int taskId) {
    for (auto& task : tasks) {
        if (task.getTaskId() == taskId) {
            return &task;
        }
    }
    return nullptr;
}

std::map<TaskStatus, int> TaskManager::getTaskStatusCount() const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::map<TaskStatus, int> counts;
    
    for (const auto& task : tasks) {
        counts[task.getStatus()]++;
    }
    return counts;
}

std::vector<Task> TaskManager::getRecentTasks(int limit) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result = tasks;
    
    if (result.size() > static_cast<size_t>(limit)) {
        result.resize(limit);
    }
    return result;
}

void TaskManager::saveToFile() const {
    try {
        std::ofstream file("data/tasks.db");
        if (file.is_open()) {
            file << nextTaskId << "\n";
            for (const auto& task : tasks) {
                file << task.getTaskId() << "|" << task.getTitle() << "|" 
                     << task.getDescription() << "|" << static_cast<int>(task.getStatus()) << "|" 
                     << static_cast<int>(task.getPriority()) << "|" << task.getAssigneeId() << "|" 
                     << task.getReporterId() << "|" << task.getProjectKey() << "\n";
            }
            file.close();
        } else {
            std::cerr << "Failed to open tasks.db for writing" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving tasks to file: " << e.what() << std::endl;
    }
}

void TaskManager::loadFromFile() {
    std::ifstream file("data/tasks.db");
    if (file.is_open()) {
        file >> nextTaskId;
        file.ignore();
        
        std::string line;
        while (std::getline(file, line)) {
            // Parse task data from file - simplified for brevity
        }
        file.close();
    }
}