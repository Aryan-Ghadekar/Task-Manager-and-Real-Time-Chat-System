#include "../include/Task.hpp"
#include <sstream>
#include <iomanip>

Task::Task(int id, const std::string& t, const std::string& desc, int reporter, const std::string& project)
    : taskId(id), title(t), description(desc), status(TaskStatus::TODO), priority(TaskPriority::MEDIUM),
      assigneeId(-1), reporterId(reporter), projectKey(project) {
    createdAt = std::chrono::system_clock::now();
    updatedAt = createdAt;
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
    updatedAt = std::chrono::system_clock::now();
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
        << " | Assignee: " << (assigneeId == -1 ? "Unassigned" : std::to_string(assigneeId));
    return oss.str();
}