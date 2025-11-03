#pragma once
#include <string>
#include <vector>
#include <chrono>

enum class TaskStatus {
    TODO,
    IN_PROGRESS,
    IN_REVIEW,
    DONE,
    BLOCKED
};

enum class TaskPriority {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

class Task {
private:
    int taskId;
    std::string title;
    std::string description;
    TaskStatus status;
    TaskPriority priority;
    int assigneeId;
    int reporterId;
    std::string projectKey;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    std::vector<std::string> comments;

public:
    Task() : taskId(-1), title(""), description(""), status(TaskStatus::TODO), priority(TaskPriority::MEDIUM), assigneeId(-1), reporterId(-1), projectKey("") {
        createdAt = std::chrono::system_clock::now();
        updatedAt = createdAt;
    }
    Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project);
    
    // Getters
    int getTaskId() const { return taskId; }
    std::string getTitle() const { return title; }
    std::string getDescription() const { return description; }
    TaskStatus getStatus() const { return status; }
    TaskPriority getPriority() const { return priority; }
    int getAssigneeId() const { return assigneeId; }
    int getReporterId() const { return reporterId; }
    std::string getProjectKey() const { return projectKey; }
    
    // Setters
    void setStatus(TaskStatus s);
    void setPriority(TaskPriority p);
    void setAssignee(int userId);
    void setTitle(const std::string& t);
    void setDescription(const std::string& desc);
    
    void addComment(const std::string& comment);
    std::vector<std::string> getComments() const { return comments; }
    
    std::string getStatusString() const;
    std::string getPriorityString() const;
    std::string toString() const;
};