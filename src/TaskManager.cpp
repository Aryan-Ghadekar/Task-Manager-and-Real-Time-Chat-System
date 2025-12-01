#include "../include/TaskManager.hpp"
#include "../include/Task.hpp"
#include "../include/NetworkUtils.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <climits>
#include <sstream>
#include <iomanip>

TaskManager::TaskManager() : nextTaskId(1) {
    loadFromFile();
}

int TaskManager::createTask(const std::string& title, const std::string& description, int reporterId, const std::string& projectKey) {
    return createTask(title, description, reporterId, projectKey, 7); // Default 7 days
}

int TaskManager::createTask(const std::string& title, const std::string& description, int reporterId, const std::string& projectKey, int deadlineDays) {
    try {
        std::lock_guard<std::mutex> lock(taskMutex);
        
        if (title.empty()) {
            throw std::invalid_argument("Task title cannot be empty");
        }
        
        Task newTask(nextTaskId, title, description, reporterId, projectKey, deadlineDays);
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

// SMART ASSIGNMENT: Recommend best assignee based on workload
int TaskManager::recommendBestAssignee(const std::map<std::string, User>& users) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    
    int bestUserId = -1;
    int minWorkload = INT_MAX;
    
    for (const auto& userPair : users) {
        const User& user = userPair.second;
        // FIX: Only consider developers for task assignment (not PMs)
        if (user.getRole() == UserRole::DEVELOPER) {
            int workload = getActiveTaskCount(user.getUserId());
            if (workload < minWorkload) {
                minWorkload = workload;
                bestUserId = user.getUserId();
            }
        }
    }
    return bestUserId;
}

int TaskManager::getActiveTaskCount(int userId) const {
    int count = 0;
    for (const auto& task : tasks) {
        if (task.getAssigneeId() == userId && 
            (task.getStatus() == TaskStatus::TODO || task.getStatus() == TaskStatus::IN_PROGRESS)) {
            count++;
        }
    }
    return count;
}

std::vector<Task> TaskManager::getOverdueTasks() const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        if (task.isOverdue() && task.getStatus() != TaskStatus::DONE) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskManager::getDueSoonTasks(int days) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        int daysLeft = task.getDaysUntilDeadline();
        if (daysLeft <= days && daysLeft >= 0 && task.getStatus() != TaskStatus::DONE) {
            result.push_back(task);
        }
    }
    return result;
}

// DASHBOARD: Generate comprehensive dashboard
std::string TaskManager::generateDashboard(const std::map<std::string, User>& users) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::ostringstream dashboard;
    
    dashboard << "\n=== PROJECT DASHBOARD ===\n";
    dashboard << "Total Tasks: " << tasks.size() << "\n\n";
    
    // Tasks by user - FIX: Direct iteration to avoid deadlock
    dashboard << "TASKS BY USER:\n";
    for (const auto& userPair : users) {
        const User& user = userPair.second;
        
        // Get user tasks directly without calling getTasksByAssignee
        std::vector<Task> userTasks;
        for (const auto& task : tasks) {
            if (task.getAssigneeId() == user.getUserId()) {
                userTasks.push_back(task);
            }
        }
        
        if (!userTasks.empty()) {
            dashboard << "  " << user.getUsername() << " (" << user.getRoleString() << "): " << userTasks.size() << " tasks\n";
            for (const auto& task : userTasks) {
                dashboard << "    - [" << task.getProjectKey() << "-" << task.getTaskId() << "] " 
                         << task.getTitle() << " (" << task.getStatusString() << ", Due: " 
                         << task.getDeadlineString() << ")\n";
            }
        }
    }
    
    // Overdue tasks - FIX: Direct iteration to avoid deadlock
    std::vector<Task> overdue;
    for (const auto& task : tasks) {
        if (task.isOverdue() && task.getStatus() != TaskStatus::DONE) {
            overdue.push_back(task);
        }
    }
    if (!overdue.empty()) {
        dashboard << "\nOVERDUE TASKS (" << overdue.size() << "):\n";
        for (const auto& task : overdue) {
            dashboard << "  - " << task.toString() << "\n";
        }
    }
    
    // Due soon - FIX: Direct iteration to avoid deadlock
    std::vector<Task> dueSoon;
    for (const auto& task : tasks) {
        int daysLeft = task.getDaysUntilDeadline();
        if (daysLeft <= 3 && daysLeft >= 0 && task.getStatus() != TaskStatus::DONE) {
            dueSoon.push_back(task);
        }
    }
    if (!dueSoon.empty()) {
        dashboard << "\nDUE SOON (Next 3 days, " << dueSoon.size() << "):\n";
        for (const auto& task : dueSoon) {
            dashboard << "  - " << task.toString() << "\n";
        }
    }
    
    return dashboard.str();
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

std::vector<Task> TaskManager::getTasksByDeadlineStatus(const std::string& status) const {
    std::lock_guard<std::mutex> lock(taskMutex);
    std::vector<Task> result;
    
    for (const auto& task : tasks) {
        if (status == "overdue" && task.isOverdue()) {
            result.push_back(task);
        } else if (status == "due_soon" && task.getDaysUntilDeadline() <= 3 && task.getDaysUntilDeadline() >= 0) {
            result.push_back(task);
        } else if (status == "completed" && task.getStatus() == TaskStatus::DONE) {
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
                // FIX: Save with deadline information
                auto deadline_time_t = std::chrono::system_clock::to_time_t(task.getDeadline());
                auto assigned_time_t = std::chrono::system_clock::to_time_t(task.getAssignedAt());
                
                file << task.getTaskId() << "|" << task.getTitle() << "|" 
                     << task.getDescription() << "|" << static_cast<int>(task.getStatus()) << "|" 
                     << static_cast<int>(task.getPriority()) << "|" << task.getAssigneeId() << "|" 
                     << task.getReporterId() << "|" << task.getProjectKey() << "|" 
                     << deadline_time_t << "|" << assigned_time_t << "\n";
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
    try {
        std::ifstream file("data/tasks.db");
        if (file.is_open()) {
            file >> nextTaskId;
            file.ignore();
            
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                
                // FIX: Proper task parsing with deadline support
                std::vector<std::string> parts = NetworkUtils::splitString(line, '|');
                if (parts.size() >= 8) {
                    try {
                        Task task;
                        // Basic fields (compatible with old format)
                        task = Task(std::stoi(parts[0]), parts[1], parts[2], 
                                   std::stoi(parts[6]), parts[7]);
                        task.setStatus(static_cast<TaskStatus>(std::stoi(parts[3])));
                        task.setPriority(static_cast<TaskPriority>(std::stoi(parts[4])));
                        if (std::stoi(parts[5]) != -1) {
                            task.setAssignee(std::stoi(parts[5]));
                        }
                        
                        // Load deadline if available (new format)
                        if (parts.size() >= 10) {
                            std::time_t deadline_time = std::stoll(parts[8]);
                            task.setDeadline(std::chrono::system_clock::from_time_t(deadline_time));
                        }
                        
                        tasks.push_back(task);
                        projectTasks[parts[7]].push_back(std::stoi(parts[0]));
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing task line: " << line << std::endl;
                    }
                }
            }
            file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading tasks from file: " << e.what() << std::endl;
    }
}