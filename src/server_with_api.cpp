// Enhanced JIRA-like Server with HTTP API - Cross-Platform
#include "../include/ChatManager.hpp"
#include "../include/HTTPServer.hpp"
#include "../include/NetworkUtils.hpp"
#include "../include/SocketAbstraction.hpp"
#include "../include/TaskManager.hpp"
#include "../include/User.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

std::mutex serverMutex;
TaskManager taskManager;
ChatManager chatManager;
std::vector<ClientInfo> clients;
std::map<std::string, User> users;
int nextUserId = 1;

void initializeUsers() {
  users.insert(
      {"admin", User(1, "admin", "admin@company.com", UserRole::ADMIN)});
  users.insert(
      {"pm1", User(2, "pm1", "pm1@company.com", UserRole::PROJECT_MANAGER)});
  users.insert(
      {"dev1", User(3, "dev1", "dev1@company.com", UserRole::DEVELOPER)});
  users.insert(
      {"tester1", User(4, "tester1", "tester1@company.com", UserRole::TESTER)});
  nextUserId = 5;
}

ClientInfo *findClient(SocketHandle socketId) {
  try {
    for (auto &client : clients) {
      if (client.socketId == socketId)
        return &client;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error finding client: " << e.what() << std::endl;
  }
  return nullptr;
}

std::vector<std::string> getOnlineUsers() {
  std::vector<std::string> onlineUsers;
  for (const auto &pair : users) {
    if (pair.second.getOnlineStatus()) {
      onlineUsers.push_back(pair.first + " (" + pair.second.getRoleString() +
                            ")");
    }
  }
  return onlineUsers;
}

bool sendSafeMessage(SocketHandle socket, const std::string &message) {
  try {
    int result =
        SocketAbstraction::sendData(socket, message.c_str(), message.size());
    return result != -1;
  } catch (const std::exception &e) {
    std::cerr << "Error sending message: " << e.what() << std::endl;
    return false;
  }
}

void processCommand(SocketHandle clientSock, const std::string &command);
void handleClient(SocketHandle clientSock);

// TCP Socket Server Thread
void runTCPServer() {
  try {
    SocketHandle serverSock = SocketAbstraction::createSocket();
    if (!SocketAbstraction::isValidSocket(serverSock)) {
      std::cerr << "Failed to create TCP socket" << std::endl;
      return;
    }

    if (!SocketAbstraction::setReuseAddress(serverSock)) {
      std::cerr << "Failed to set socket options" << std::endl;
      SocketAbstraction::closeSocket(serverSock);
      return;
    }

    if (!SocketAbstraction::bindSocket(serverSock, 8080)) {
      std::cerr << "TCP Bind failed" << std::endl;
      SocketAbstraction::closeSocket(serverSock);
      return;
    }

    if (!SocketAbstraction::listenSocket(serverSock, 10)) {
      std::cerr << "TCP Listen failed" << std::endl;
      SocketAbstraction::closeSocket(serverSock);
      return;
    }

    std::cout << "TCP Socket Server running on port 8080..." << std::endl;

    while (true) {
      SocketHandle clientSock = SocketAbstraction::acceptSocket(serverSock);
      if (SocketAbstraction::isValidSocket(clientSock)) {
        std::cout << "New TCP client connected" << std::endl;
        std::thread(handleClient, clientSock).detach();
      }
    }

    SocketAbstraction::closeSocket(serverSock);
  } catch (const std::exception &e) {
    std::cerr << "TCP Server error: " << e.what() << std::endl;
  }
}

// HTTP API Server Thread
void runHTTPServer() {
  try {
    HTTPServer httpServer(taskManager, chatManager, users, serverMutex);
    httpServer.setupRoutes();
    httpServer.start(8081); // HTTP API on port 8081
  } catch (const std::exception &e) {
    std::cerr << "HTTP Server error: " << e.what() << std::endl;
  }
}

int main() {
  try {
    // Initialize socket subsystem (required on Windows)
    if (!SocketAbstraction::initialize()) {
      std::cerr << "Failed to initialize socket subsystem" << std::endl;
      return -1;
    }

    initializeUsers();

    std::cout << "=== JIRA-like Task Manager - Dual Server Mode ==="
              << std::endl;
    std::cout << "TCP Socket Server: port 8080 (for CLI clients)" << std::endl;
    std::cout << "HTTP API Server: port 8081 (for frontend)" << std::endl;
    std::cout << "Available users: admin, pm1, dev1, tester1" << std::endl;
    std::cout << "================================================="
              << std::endl;

    // Start TCP server in separate thread
    std::thread tcpThread(runTCPServer);

    // Start HTTP server in main thread (blocking)
    runHTTPServer();

    tcpThread.join();

    SocketAbstraction::cleanup();
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << std::endl;
    SocketAbstraction::cleanup();
    return -1;
  }

  return 0;
}

// Include the original command processing and client handling code
#include "server.cpp"
