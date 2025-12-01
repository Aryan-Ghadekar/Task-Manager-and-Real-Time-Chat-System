#include "../include/Task.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

Task::Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project)
    : taskId(id), title(t), description(desc), status(TaskStatus::TODO), priority(TaskPriority::MEDIUM),
      assigneeId(-1), reporterId(reporter), projectKey(project) {
    createdAt = std::chrono::system_clock::now();
    updatedAt = createdAt;
    deadline = createdAt + std::chrono::hours(24 * 7); // Default 7 days
    assignedAt = std::chrono::system_clock::time_point{};
}

Task::Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project, int deadlineDays)
    : taskId(id), title(t), description(desc), status(TaskStatus::TODO), priority(TaskPriority::MEDIUM),
      assigneeId(-1), reporterId(reporter), projectKey(project) {
    createdAt = std::chrono::system_clock::now();
    updatedAt = createdAt;
    deadline = createdAt + std::chrono::hours(24 * deadlineDays);
    assignedAt = std::chrono::system_clock::time_point{};
}

void Task::setStatus(TaskStatus s) {
    status = s;
    updatedAt = std::chrono::system_clock::now();
}

void Task::setPriority(TaskPriority p) {
    priority = p;
    updatedAt = std::chrono::system_clock::now();
}

void Task::setAssignee(int userId) {
    assigneeId = userId;
    assignedAt = std::chrono::system_clock::now(); // Track assignment time
    updatedAt = assignedAt;
}

void Task::setTitle(const std::string& t) {
    title = t;
    updatedAt = std::chrono::system_clock::now();
}

void Task::setDescription(const std::string& desc) {
    description = desc;
    updatedAt = std::chrono::system_clock::now();
}

void Task::addComment(const std::string& comment) {
    comments.push_back(comment);
    updatedAt = std::chrono::system_clock::now();
}

std::string Task::getStatusString() const {
    switch (status) {
        case TaskStatus::TODO: return "To Do";
        case TaskStatus::IN_PROGRESS: return "In Progress";
        case TaskStatus::IN_REVIEW: return "In Review";
        case TaskStatus::DONE: return "Done";
        case TaskStatus::BLOCKED: return "Blocked";
        default: return "Unknown";
    }
}

std::string Task::getPriorityString() const {
    switch (priority) {
        case TaskPriority::LOW: return "Low";
        case TaskPriority::MEDIUM: return "Medium";
        case TaskPriority::HIGH: return "High";
        case TaskPriority::CRITICAL: return "Critical";
        default: return "Unknown";
    }
}

std::string Task::toString() const {
    std::ostringstream oss;
    oss << "[" << projectKey << "-" << taskId << "] " << title
        << " | Status: " << getStatusString()
        << " | Priority: " << getPriorityString()
        << " | Assignee: " << (assigneeId == -1 ? "Unassigned" : std::to_string(assigneeId))
        << " | Deadline: " << getDeadlineString()
        << " | Days Left: " << getDaysUntilDeadline();
    return oss.str();
}

std::string Task::getDeadlineString() const {
    auto time_t = std::chrono::system_clock::to_time_t(deadline);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
    return oss.str();
}

bool Task::isOverdue() const {
    return std::chrono::system_clock::now() > deadline;
}

int Task::getDaysUntilDeadline() const {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::hours>(deadline - now);
    return static_cast<int>(diff.count() / 24);
}