// Enhanced JIRA-like Client with Error Handling - Cross-Platform
#include "../include/NetworkUtils.hpp"
#include "../include/SocketAbstraction.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

std::string username;
int userId = -1;
bool authenticated = false;

void receiveMessages(SocketHandle sock) {
  char buffer[1024];
  while (true) {
    try {
      int bytes =
          SocketAbstraction::receiveData(sock, buffer, sizeof(buffer) - 1);
      if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << "\n" << buffer << "\n> ";
        std::cout.flush();
      } else if (bytes == 0) {
        std::cout << "\n[SYSTEM] Connection closed by server\n";
        break;
      } else {
        std::cerr << "\n[ERROR] Read error: "
                  << SocketAbstraction::getLastError() << "\n";
        break;
      }
    } catch (const std::exception &e) {
      std::cerr << "\n[ERROR] Exception in receive: " << e.what() << "\n";
      break;
    }
  }
}

void showHelp() {
  std::cout << "\n============ Task Manager Commands ================\n";
  std::cout << "AUTHENTICATION:\n";
  std::cout << "  /login <username> <password>  - Login to system\n\n";

  std::cout << "TASK MANAGEMENT:\n";
  std::cout
      << "  /create <title> | <desc> [deadline:X] - Create task (X=days)\n";
  std::cout
      << "  /assign <taskId> <userId>     - Assign task (PM/Admin only)\n";
  std::cout << "  /status <taskId> <status>     - Update status "
               "(TODO/PROGRESS/REVIEW/DONE/BLOCKED)\n";
  std::cout << "  /priority <taskId> <priority> - Set priority "
               "(LOW/MEDIUM/HIGH/CRITICAL)\n";
  std::cout << "  /comment <taskId> <comment>   - Add comment to task\n";
  std::cout
      << "  /list                         - List all tasks with deadlines\n";
  std::cout << "  /mytasks                      - Show my assigned tasks\n\n";

  std::cout << "SMART FEATURES:\n";
  std::cout << "  /dashboard                    - Show project dashboard\n";
  std::cout
      << "  /recommend                    - Get best assignee recommendation\n";
  std::cout << "  /overdue                      - Show overdue tasks\n\n";

  std::cout << "COMMUNICATION:\n";
  std::cout << "  /chat <message>               - Send public message\n";
  std::cout << "  /pm <user/userId> <message>   - Send private message (not "
               "logged)\n";
  std::cout << "  /online                       - Show online users\n\n";

  std::cout << "SYSTEM:\n";
  std::cout << "  /help                         - Show this help\n";
  std::cout << "  /quit                         - Exit application\n";
  std::cout << "================================================\n\n";
}

int main() {
  try {
    // Initialize socket subsystem (required on Windows)
    if (!SocketAbstraction::initialize()) {
      std::cerr << "Failed to initialize socket subsystem" << std::endl;
      return -1;
    }

    SocketHandle sock = SocketAbstraction::createSocket();
    if (!SocketAbstraction::isValidSocket(sock)) {
      std::cerr << "Failed to create socket: "
                << SocketAbstraction::getLastError() << std::endl;
      SocketAbstraction::cleanup();
      return -1;
    }

    if (!SocketAbstraction::connectSocket(sock, "127.0.0.1", 8080)) {
      std::cerr << "Failed to connect to server: "
                << SocketAbstraction::getLastError() << std::endl;
      SocketAbstraction::closeSocket(sock);
      SocketAbstraction::cleanup();
      return -1;
    }

    std::thread receiver(receiveMessages, sock);
    receiver.detach();

    std::cout << "Connected to the Task Manager\n";
    showHelp();

    std::string input;
    while (true) {
      std::cout << "> ";
      if (!std::getline(std::cin, input)) {
        break; // EOF or error
      }

      if (input == "/quit")
        break;
      if (input == "/help") {
        showHelp();
        continue;
      }

      if (input.empty())
        continue;

      try {
        std::string message = NetworkUtils::formatMessage("CMD", input);
        int sent =
            SocketAbstraction::sendData(sock, message.c_str(), message.size());
        if (sent < 0) {
          std::cerr << "Failed to send message: "
                    << SocketAbstraction::getLastError() << std::endl;
          break;
        }
      } catch (const std::exception &e) {
        std::cerr << "Error sending command: " << e.what() << std::endl;
      }
    }

    SocketAbstraction::closeSocket(sock);
    SocketAbstraction::cleanup();
  } catch (const std::exception &e) {
    std::cerr << "Client error: " << e.what() << std::endl;
    SocketAbstraction::cleanup();
    return -1;
  }

  return 0;
}
