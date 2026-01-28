#include "../include/HTTPServer.hpp"
#include "../include/NetworkUtils.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

HTTPServer::HTTPServer(TaskManager &tm, ChatManager &cm,
                       std::map<std::string, User> &u, std::mutex &m)
    : taskManager(tm), chatManager(cm), users(u), serverMutex(m) {}

std::string HTTPServer::generateToken(const std::string &username) {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << "TOKEN_" << username << "_" << time_t;
  return oss.str();
}

bool HTTPServer::validateToken(const std::string &token,
                               std::string &username) {
  std::lock_guard<std::mutex> lock(sessionMutex);
  auto it = sessions.find(token);
  if (it != sessions.end()) {
    username = it->second;
    return true;
  }
  return false;
}

std::string HTTPServer::getUsernameFromToken(const std::string &token) {
  std::lock_guard<std::mutex> lock(sessionMutex);
  auto it = sessions.find(token);
  return (it != sessions.end()) ? it->second : "";
}

std::string HTTPServer::taskToJSON(const Task &task) {
  std::ostringstream oss;
  oss << "{"
      << "\"id\":" << task.getTaskId() << ","
      << "\"title\":\"" << task.getTitle() << "\","
      << "\"description\":\"" << task.getDescription() << "\","
      << "\"status\":\"" << task.getStatusString() << "\","
      << "\"priority\":\"" << task.getPriorityString() << "\","
      << "\"assigneeId\":" << task.getAssigneeId() << ","
      << "\"reporterId\":" << task.getReporterId() << ","
      << "\"projectKey\":\"" << task.getProjectKey() << "\","
      << "\"deadline\":\"" << task.getDeadlineString() << "\","
      << "\"isOverdue\":" << (task.isOverdue() ? "true" : "false") << ","
      << "\"daysUntilDeadline\":" << task.getDaysUntilDeadline() << "}";
  return oss.str();
}

std::string HTTPServer::tasksToJSON(const std::vector<Task> &tasks) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < tasks.size(); ++i) {
    oss << taskToJSON(tasks[i]);
    if (i < tasks.size() - 1)
      oss << ",";
  }
  oss << "]";
  return oss.str();
}

std::string HTTPServer::chatToJSON(const Chat &chat) {
  std::ostringstream oss;
  oss << "{"
      << "\"id\":" << chat.getMessageId() << ","
      << "\"senderId\":" << chat.getSenderId() << ","
      << "\"senderName\":\"" << chat.getSenderName() << "\","
      << "\"content\":\"" << chat.getContent() << "\","
      << "\"type\":\"" << chat.getTypeString() << "\","
      << "\"timestamp\":\"" << chat.getTimestamp() << "\","
      << "\"targetUserId\":" << chat.getTargetUserId() << ","
      << "\"relatedTaskId\":" << chat.getRelatedTaskId() << "}";
  return oss.str();
}

std::string HTTPServer::chatsToJSON(const std::vector<Chat> &chats) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < chats.size(); ++i) {
    oss << chatToJSON(chats[i]);
    if (i < chats.size() - 1)
      oss << ",";
  }
  oss << "]";
  return oss.str();
}

std::string HTTPServer::userToJSON(const User &user,
                                   const std::string &username) {
  std::ostringstream oss;
  oss << "{"
      << "\"id\":" << user.getUserId() << ","
      << "\"username\":\"" << username << "\","
      << "\"email\":\"" << user.getEmail() << "\","
      << "\"role\":\"" << user.getRoleString() << "\","
      << "\"isOnline\":" << (user.getOnlineStatus() ? "true" : "false") << "}";
  return oss.str();
}

std::string HTTPServer::usersToJSON(const std::map<std::string, User> &users) {
  std::ostringstream oss;
  oss << "[";
  size_t i = 0;
  for (const auto &pair : users) {
    oss << userToJSON(pair.second, pair.first);
    if (i < users.size() - 1)
      oss << ",";
    ++i;
  }
  oss << "]";
  return oss.str();
}

std::string HTTPServer::errorJSON(const std::string &message) {
  return "{\"success\":false,\"error\":\"" + message + "\"}";
}

std::string HTTPServer::successJSON(const std::string &message,
                                    const std::string &data) {
  std::string result = "{\"success\":true,\"message\":\"" + message + "\"";
  if (!data.empty()) {
    result += ",\"data\":" + data;
  }
  result += "}";
  return result;
}

void HTTPServer::setupRoutes() {
  // Enable CORS for frontend access
  server.set_default_headers(
      {{"Access-Control-Allow-Origin", "*"},
       {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
       {"Access-Control-Allow-Headers", "Content-Type, Authorization"}});

  // Handle OPTIONS requests for CORS preflight
  server.Options(".*", [](const httplib::Request &, httplib::Response &res) {
    res.set_content("", "text/plain");
  });

  // ===== AUTHENTICATION =====

  // POST /api/login - User login
  server.Post("/api/login", [this](const httplib::Request &req,
                                   httplib::Response &res) {
    std::lock_guard<std::mutex> lock(serverMutex);

    // Parse JSON body (simple parsing for username/password)
    std::string body = req.body;
    size_t userPos = body.find("\"username\":\"");
    size_t passPos = body.find("\"password\":\"");

    if (userPos == std::string::npos || passPos == std::string::npos) {
      res.set_content(errorJSON("Invalid request format"), "application/json");
      return;
    }

    userPos += 12; // length of "\"username\":\""
    size_t userEnd = body.find("\"", userPos);
    std::string username = body.substr(userPos, userEnd - userPos);

    passPos += 12; // length of "\"password\":\""
    size_t passEnd = body.find("\"", passPos);
    std::string password = body.substr(passPos, passEnd - passPos);

    if (users.find(username) != users.end() &&
        NetworkUtils::authenticateUser(username, password)) {
      std::string token = generateToken(username);
      {
        std::lock_guard<std::mutex> sessLock(sessionMutex);
        sessions[token] = username;
      }
      users[username].setOnlineStatus(true);

      std::string userData = userToJSON(users[username], username);
      std::string response = "{\"success\":true,\"token\":\"" + token +
                             "\",\"user\":" + userData + "}";
      res.set_content(response, "application/json");
    } else {
      res.set_content(errorJSON("Invalid credentials"), "application/json");
    }
  });

  // POST /api/logout - User logout
  server.Post("/api/logout", [this](const httplib::Request &req,
                                    httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ") {
      token = token.substr(7);
    }

    std::string username = getUsernameFromToken(token);
    if (!username.empty()) {
      std::lock_guard<std::mutex> lock(serverMutex);
      users[username].setOnlineStatus(false);
      {
        std::lock_guard<std::mutex> sessLock(sessionMutex);
        sessions.erase(token);
      }
      res.set_content(successJSON("Logged out successfully"),
                      "application/json");
    } else {
      res.set_content(errorJSON("Invalid token"), "application/json");
    }
  });

  // ===== TASKS =====

  // GET /api/tasks - Get all tasks
  server.Get("/api/tasks", [this](const httplib::Request &req,
                                  httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    auto tasks = taskManager.getAllTasks();
    res.set_content(successJSON("Tasks retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // GET /api/tasks/my - Get my tasks
  server.Get("/api/tasks/my", [this](const httplib::Request &req,
                                     httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    auto tasks = taskManager.getTasksByAssignee(userId);
    res.set_content(successJSON("My tasks retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // GET /api/tasks/overdue - Get overdue tasks
  server.Get("/api/tasks/overdue", [this](const httplib::Request &req,
                                          httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    auto tasks = taskManager.getOverdueTasks();
    res.set_content(successJSON("Overdue tasks retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // POST /api/tasks - Create task
  server.Post("/api/tasks", [this](const httplib::Request &req,
                                   httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    // Simple JSON parsing for title, description, deadline
    std::string body = req.body;
    size_t titlePos = body.find("\"title\":\"");
    size_t descPos = body.find("\"description\":\"");
    size_t deadlinePos = body.find("\"deadlineDays\":");

    if (titlePos == std::string::npos) {
      res.set_content(errorJSON("Title is required"), "application/json");
      return;
    }

    titlePos += 9;
    size_t titleEnd = body.find("\"", titlePos);
    std::string title = body.substr(titlePos, titleEnd - titlePos);

    std::string description = "";
    if (descPos != std::string::npos) {
      descPos += 15;
      size_t descEnd = body.find("\"", descPos);
      description = body.substr(descPos, descEnd - descPos);
    }

    int deadlineDays = 7;
    if (deadlinePos != std::string::npos) {
      deadlinePos += 15;
      size_t deadlineEnd = body.find_first_of(",}", deadlinePos);
      deadlineDays =
          std::stoi(body.substr(deadlinePos, deadlineEnd - deadlinePos));
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    int taskId = taskManager.createTask(title, description, userId, "PROJ",
                                        deadlineDays);

    Task *task = taskManager.getTaskById(taskId);
    if (task) {
      res.set_content(successJSON("Task created", taskToJSON(*task)),
                      "application/json");
    } else {
      res.set_content(errorJSON("Failed to create task"), "application/json");
    }
  });

  // PUT /api/tasks/:id/status - Update task status
  server.Put(R"(/api/tasks/(\d+)/status)", [this](const httplib::Request &req,
                                                  httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::string body = req.body;
    size_t statusPos = body.find("\"status\":\"");
    if (statusPos == std::string::npos) {
      res.set_content(errorJSON("Status is required"), "application/json");
      return;
    }

    statusPos += 10;
    size_t statusEnd = body.find("\"", statusPos);
    std::string statusStr = body.substr(statusPos, statusEnd - statusPos);

    TaskStatus status = TaskStatus::TODO;
    if (statusStr == "IN_PROGRESS")
      status = TaskStatus::IN_PROGRESS;
    else if (statusStr == "IN_REVIEW")
      status = TaskStatus::IN_REVIEW;
    else if (statusStr == "DONE")
      status = TaskStatus::DONE;
    else if (statusStr == "BLOCKED")
      status = TaskStatus::BLOCKED;

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    if (taskManager.updateTaskStatus(taskId, status, userId)) {
      Task *task = taskManager.getTaskById(taskId);
      res.set_content(successJSON("Status updated", taskToJSON(*task)),
                      "application/json");
    } else {
      res.set_content(errorJSON("Failed to update status"), "application/json");
    }
  });

  // ===== CHAT =====

  // GET /api/chat - Get recent messages
  server.Get("/api/chat", [this](const httplib::Request &req,
                                 httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    auto messages = chatManager.getRecentMessages(50);
    res.set_content(successJSON("Messages retrieved", chatsToJSON(messages)),
                    "application/json");
  });

  // POST /api/chat - Send message
  server.Post(
      "/api/chat", [this](const httplib::Request &req, httplib::Response &res) {
        std::string token = req.get_header_value("Authorization");
        if (token.substr(0, 7) == "Bearer ")
          token = token.substr(7);

        std::string username;
        if (!validateToken(token, username)) {
          res.set_content(errorJSON("Unauthorized"), "application/json");
          return;
        }

        std::string body = req.body;
        size_t contentPos = body.find("\"content\":\"");
        if (contentPos == std::string::npos) {
          res.set_content(errorJSON("Content is required"), "application/json");
          return;
        }

        contentPos += 11;
        size_t contentEnd = body.find("\"", contentPos);
        std::string content = body.substr(contentPos, contentEnd - contentPos);

        std::lock_guard<std::mutex> lock(serverMutex);
        int userId = users[username].getUserId();
        int messageId = chatManager.sendMessage(userId, username, content);

        res.set_content(
            successJSON("Message sent",
                        "{\"messageId\":" + std::to_string(messageId) + "}"),
            "application/json");
      });

  // ===== USERS =====

  // GET /api/users/online - Get online users
  server.Get("/api/users/online", [this](const httplib::Request &req,
                                         httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    res.set_content(successJSON("Online users retrieved", usersToJSON(users)),
                    "application/json");
  });

  // GET /api/dashboard - Get dashboard data
  server.Get("/api/dashboard", [this](const httplib::Request &req,
                                      httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    std::string dashboard = taskManager.generateDashboard(users);
    res.set_content(successJSON("Dashboard data retrieved",
                                "{\"dashboard\":\"" + dashboard + "\"}"),
                    "application/json");
  });

  // ===== PRIORITY MANAGEMENT =====

  // PUT /api/tasks/:id/priority - Update task priority
  server.Put(R"(/api/tasks/(\d+)/priority)", [this](const httplib::Request &req,
                                                     httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::string body = req.body;
    size_t priorityPos = body.find("\"priority\":\"");
    if (priorityPos == std::string::npos) {
      res.set_content(errorJSON("Priority is required"), "application/json");
      return;
    }

    priorityPos += 12;
    size_t priorityEnd = body.find("\"", priorityPos);
    std::string priorityStr = body.substr(priorityPos, priorityEnd - priorityPos);

    TaskPriority priority = TaskPriority::MEDIUM;
    if (priorityStr == "LOW")
      priority = TaskPriority::LOW;
    else if (priorityStr == "HIGH")
      priority = TaskPriority::HIGH;
    else if (priorityStr == "CRITICAL")
      priority = TaskPriority::CRITICAL;

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    if (taskManager.updateTaskPriority(taskId, priority, userId)) {
      Task *task = taskManager.getTaskById(taskId);
      res.set_content(successJSON("Priority updated", taskToJSON(*task)),
                      "application/json");
    } else {
      res.set_content(errorJSON("Failed to update priority"), "application/json");
    }
  });

  // ===== TASK ASSIGNMENT =====

  // PUT /api/tasks/:id/assign - Assign task to user
  server.Put(R"(/api/tasks/(\d+)/assign)", [this](const httplib::Request &req,
                                                   httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::string body = req.body;
    size_t assigneePos = body.find("\"assigneeId\":");
    if (assigneePos == std::string::npos) {
      res.set_content(errorJSON("Assignee ID is required"), "application/json");
      return;
    }

    assigneePos += 14;
    size_t assigneeEnd = body.find_first_of(",}", assigneePos);
    int assigneeId = std::stoi(body.substr(assigneePos, assigneeEnd - assigneePos));

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    if (taskManager.assignTask(taskId, assigneeId, userId)) {
      Task *task = taskManager.getTaskById(taskId);
      res.set_content(successJSON("Task assigned", taskToJSON(*task)),
                      "application/json");
    } else {
      res.set_content(errorJSON("Failed to assign task"), "application/json");
    }
  });

  // ===== TASK COMMENTS =====

  // GET /api/tasks/:id/comments - Get task comments
  server.Get(R"(/api/tasks/(\d+)/comments)", [this](const httplib::Request &req,
                                                     httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::lock_guard<std::mutex> lock(serverMutex);
    Task *task = taskManager.getTaskById(taskId);
    if (!task) {
      res.set_content(errorJSON("Task not found"), "application/json");
      return;
    }

    auto comments = task->getComments();
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < comments.size(); ++i) {
      oss << "\"" << comments[i] << "\"";
      if (i < comments.size() - 1)
        oss << ",";
    }
    oss << "]";

    res.set_content(successJSON("Comments retrieved", oss.str()),
                    "application/json");
  });

  // POST /api/tasks/:id/comments - Add task comment
  server.Post(R"(/api/tasks/(\d+)/comments)", [this](const httplib::Request &req,
                                                      httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::string body = req.body;
    size_t commentPos = body.find("\"comment\":\"");
    if (commentPos == std::string::npos) {
      res.set_content(errorJSON("Comment is required"), "application/json");
      return;
    }

    commentPos += 11;
    size_t commentEnd = body.find("\"", commentPos);
    std::string comment = body.substr(commentPos, commentEnd - commentPos);

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    if (taskManager.addTaskComment(taskId, comment, userId)) {
      res.set_content(successJSON("Comment added"), "application/json");
    } else {
      res.set_content(errorJSON("Failed to add comment"), "application/json");
    }
  });

  // ===== ADVANCED FILTERING =====

  // GET /api/tasks/due-soon - Get tasks due soon
  server.Get("/api/tasks/due-soon", [this](const httplib::Request &req,
                                           httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int days = 3; // default
    if (req.has_param("days")) {
      days = std::stoi(req.get_param_value("days"));
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    auto tasks = taskManager.getDueSoonTasks(days);
    res.set_content(successJSON("Due soon tasks retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // GET /api/tasks/by-status/:status - Get tasks by status
  server.Get(R"(/api/tasks/by-status/([A-Z_]+))", [this](const httplib::Request &req,
                                                         httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::string statusStr = req.matches[1];
    TaskStatus status = TaskStatus::TODO;
    if (statusStr == "IN_PROGRESS")
      status = TaskStatus::IN_PROGRESS;
    else if (statusStr == "IN_REVIEW")
      status = TaskStatus::IN_REVIEW;
    else if (statusStr == "DONE")
      status = TaskStatus::DONE;
    else if (statusStr == "BLOCKED")
      status = TaskStatus::BLOCKED;

    std::lock_guard<std::mutex> lock(serverMutex);
    auto tasks = taskManager.getTasksByStatus(status);
    res.set_content(successJSON("Tasks by status retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // GET /api/tasks/by-project/:project - Get tasks by project
  server.Get(R"(/api/tasks/by-project/([A-Z0-9]+))", [this](const httplib::Request &req,
                                                             httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::string project = req.matches[1];

    std::lock_guard<std::mutex> lock(serverMutex);
    auto tasks = taskManager.getTasksByProject(project);
    res.set_content(successJSON("Tasks by project retrieved", tasksToJSON(tasks)),
                    "application/json");
  });

  // ===== SMART ASSIGNMENT =====

  // GET /api/tasks/recommend-assignee - Get recommended assignee
  server.Get("/api/tasks/recommend-assignee", [this](const httplib::Request &req,
                                                     httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    int recommendedId = taskManager.recommendBestAssignee(users);
    
    if (recommendedId == -1) {
      res.set_content(errorJSON("No suitable assignee found"), "application/json");
      return;
    }

    // Find username for recommended user
    std::string recommendedUsername;
    for (const auto &pair : users) {
      if (pair.second.getUserId() == recommendedId) {
        recommendedUsername = pair.first;
        break;
      }
    }

    int workload = taskManager.getActiveTaskCount(recommendedId);
    std::ostringstream oss;
    oss << "{\"userId\":" << recommendedId
        << ",\"username\":\"" << recommendedUsername << "\""
        << ",\"workload\":" << workload << "}";

    res.set_content(successJSON("Recommended assignee", oss.str()),
                    "application/json");
  });

  // ===== PRIVATE MESSAGING =====

  // POST /api/chat/private - Send private message
  server.Post("/api/chat/private", [this](const httplib::Request &req,
                                          httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::string body = req.body;
    size_t targetPos = body.find("\"targetUserId\":");
    size_t contentPos = body.find("\"content\":\"");

    if (targetPos == std::string::npos || contentPos == std::string::npos) {
      res.set_content(errorJSON("Target user ID and content are required"),
                      "application/json");
      return;
    }

    targetPos += 15;
    size_t targetEnd = body.find_first_of(",}", targetPos);
    int targetUserId = std::stoi(body.substr(targetPos, targetEnd - targetPos));

    contentPos += 11;
    size_t contentEnd = body.find("\"", contentPos);
    std::string content = body.substr(contentPos, contentEnd - contentPos);

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    int messageId = chatManager.sendPrivateMessage(userId, username, targetUserId, content);

    res.set_content(successJSON("Private message sent",
                                "{\"messageId\":" + std::to_string(messageId) + "}"),
                    "application/json");
  });

  // GET /api/chat/private/:userId - Get private messages with user
  server.Get(R"(/api/chat/private/(\d+))", [this](const httplib::Request &req,
                                                  httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int otherUserId = std::stoi(req.matches[1]);

    std::lock_guard<std::mutex> lock(serverMutex);
    int userId = users[username].getUserId();
    auto messages = chatManager.getPrivateMessages(userId, otherUserId);
    res.set_content(successJSON("Private messages retrieved", chatsToJSON(messages)),
                    "application/json");
  });

  // ===== TASK MESSAGES =====

  // GET /api/tasks/:id/messages - Get task-related messages
  server.Get(R"(/api/tasks/(\d+)/messages)", [this](const httplib::Request &req,
                                                    httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    int taskId = std::stoi(req.matches[1]);

    std::lock_guard<std::mutex> lock(serverMutex);
    auto messages = chatManager.getTaskMessages(taskId);
    res.set_content(successJSON("Task messages retrieved", chatsToJSON(messages)),
                    "application/json");
  });

  // ===== STATISTICS =====

  // GET /api/stats - Get comprehensive statistics
  server.Get("/api/stats", [this](const httplib::Request &req,
                                  httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    
    // Get status counts
    auto statusCounts = taskManager.getTaskStatusCount();
    int todoCount = statusCounts[TaskStatus::TODO];
    int inProgressCount = statusCounts[TaskStatus::IN_PROGRESS];
    int inReviewCount = statusCounts[TaskStatus::IN_REVIEW];
    int doneCount = statusCounts[TaskStatus::DONE];
    int blockedCount = statusCounts[TaskStatus::BLOCKED];
    int totalCount = todoCount + inProgressCount + inReviewCount + doneCount + blockedCount;

    // Get overdue and due soon counts
    int overdueCount = taskManager.getOverdueTasks().size();
    int dueSoonCount = taskManager.getDueSoonTasks(3).size();

    std::ostringstream oss;
    oss << "{"
        << "\"total\":" << totalCount << ","
        << "\"todo\":" << todoCount << ","
        << "\"inProgress\":" << inProgressCount << ","
        << "\"inReview\":" << inReviewCount << ","
        << "\"done\":" << doneCount << ","
        << "\"blocked\":" << blockedCount << ","
        << "\"overdue\":" << overdueCount << ","
        << "\"dueSoon\":" << dueSoonCount
        << "}";

    res.set_content(successJSON("Statistics retrieved", oss.str()),
                    "application/json");
  });

  // ===== USER MANAGEMENT =====

  // GET /api/users - Get all users
  server.Get("/api/users", [this](const httplib::Request &req,
                                  httplib::Response &res) {
    std::string token = req.get_header_value("Authorization");
    if (token.substr(0, 7) == "Bearer ")
      token = token.substr(7);

    std::string username;
    if (!validateToken(token, username)) {
      res.set_content(errorJSON("Unauthorized"), "application/json");
      return;
    }

    std::lock_guard<std::mutex> lock(serverMutex);
    res.set_content(successJSON("Users retrieved", usersToJSON(users)),
                    "application/json");
  });
}

void HTTPServer::start(int port) {
  std::cout << "Starting HTTP API server on port " << port << "..."
            << std::endl;
  server.listen("0.0.0.0", port);
}

void HTTPServer::stop() { server.stop(); }
