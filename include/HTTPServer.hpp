#pragma once

#include "ChatManager.hpp"
#include "TaskManager.hpp"
#include "User.hpp"
#include "httplib.h"
#include <map>
#include <mutex>
#include <string>
#include <vector>

/**
 * HTTP/REST API Server for frontend communication
 * Runs alongside the TCP socket server on port 8081
 */
class HTTPServer {
private:
  httplib::Server server;
  TaskManager &taskManager;
  ChatManager &chatManager;
  std::map<std::string, User> &users;
  std::mutex &serverMutex;

  // Session management
  std::map<std::string, std::string> sessions; // token -> username
  std::mutex sessionMutex;

  // Helper methods
  std::string generateToken(const std::string &username);
  bool validateToken(const std::string &token, std::string &username);
  std::string getUsernameFromToken(const std::string &token);

  // JSON helpers
  std::string taskToJSON(const Task &task);
  std::string tasksToJSON(const std::vector<Task> &tasks);
  std::string chatToJSON(const Chat &chat);
  std::string chatsToJSON(const std::vector<Chat> &chats);
  std::string userToJSON(const User &user, const std::string &username);
  std::string usersToJSON(const std::map<std::string, User> &users);
  std::string errorJSON(const std::string &message);
  std::string successJSON(const std::string &message,
                          const std::string &data = "");

public:
  HTTPServer(TaskManager &tm, ChatManager &cm, std::map<std::string, User> &u,
             std::mutex &m);

  /**
   * Setup all API routes
   */
  void setupRoutes();

  /**
   * Start the HTTP server (blocking call)
   * @param port Port to listen on (default: 8081)
   */
  void start(int port = 8081);

  /**
   * Stop the HTTP server
   */
  void stop();
};
