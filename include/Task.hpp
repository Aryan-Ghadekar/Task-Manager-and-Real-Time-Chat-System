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
    std::chrono::system_clock::time_point deadline;  // NEW: Task deadline
    std::chrono::system_clock::time_point assignedAt; // NEW: Assignment timestamp
    std::vector<std::string> comments;

public:
    Task() : taskId(-1), title(""), description(""), status(TaskStatus::TODO), priority(TaskPriority::MEDIUM), assigneeId(-1), reporterId(-1), projectKey("") {
        createdAt = std::chrono::system_clock::now();
        updatedAt = createdAt;
        deadline = createdAt + std::chrono::hours(24 * 7); // Default 7 days
        assignedAt = std::chrono::system_clock::time_point{};
    }
    Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project);
    Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project, int deadlineDays);
    
    // Getters
    int getTaskId() const { return taskId; }
    std::string getTitle() const { return title; }
    std::string getDescription() const { return description; }
    TaskStatus getStatus() const { return status; }
    TaskPriority getPriority() const { return priority; }
    int getAssigneeId() const { return assigneeId; }
    int getReporterId() const { return reporterId; }
    std::string getProjectKey() const { return projectKey; }
    std::chrono::system_clock::time_point getDeadline() const { return deadline; }
    std::chrono::system_clock::time_point getAssignedAt() const { return assignedAt; }
    std::string getDeadlineString() const;
    bool isOverdue() const;
    int getDaysUntilDeadline() const;
    
    // Setters
    void setStatus(TaskStatus s);
    void setPriority(TaskPriority p);
    void setAssignee(int userId);
    void setDeadline(const std::chrono::system_clock::time_point& d) { deadline = d; }
    void setDeadlineDays(int days) { deadline = std::chrono::system_clock::now() + std::chrono::hours(24 * days); }
    void setTitle(const std::string& t);
    void setDescription(const std::string& desc);
    
    void addComment(const std::string& comment);
    std::vector<std::string> getComments() const { return comments; }
    
    std::string getStatusString() const;
    std::string getPriorityString() const;
    std::string toString() const;
};