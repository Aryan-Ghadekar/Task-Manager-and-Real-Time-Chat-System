# Task Manager and Real-Time Chat System

A **C++ OOP-based project** that combines **real-time task management** with a **chat system**. Built with **socket programming, multithreading, and OOP principles**, this project allows multiple users to connect to a central server, manage tasks collaboratively, and chat in real time.

---

## ✨ Features

- 🔗 **Real-time Communication** – Multiple clients connected via TCP sockets
- 📋 **Task Management** – Add and manage tasks collaboratively using `/task` commands
- 💬 **Chat System** – Send and receive messages instantly using `/chat` commands
- ⚡ **Multithreaded Server** – Each client handled by a separate thread for concurrent connections
- 🔒 **Thread Safety** – Mutex ensures safe concurrent updates across all clients
- 📡 **Broadcast Updates** – All connected clients receive real-time updates
- 💾 **Data Persistence** – Chat logs and task data stored in the `data/` directory
- 🏗️ **Modular Architecture** – Clean separation of concerns with dedicated managers

---

## 📂 Project Structure

```
Task-Manager-and-Real-Time-Chat-System/
├── include/                    # Header files (OOP classes)
│   ├── User.hpp               # User management
│   ├── Task.hpp               # Task entity definition
│   ├── Chat.hpp               # Chat message handling
│   ├── TaskManager.hpp        # Task operations manager
│   ├── ChatManager.hpp        # Chat operations manager
│   └── NetworkUtils.hpp       # Network utility functions
│
├── src/                       # Implementation files
│   ├── server.cpp             # Server entry point with socket handling
│   ├── client.cpp             # Client entry point with user interface
│   ├── Task.cpp               # Task class implementation
│   ├── Chat.cpp               # Chat class implementation
│   ├── TaskManager.cpp        # Task management logic
│   ├── ChatManager.cpp        # Chat management logic
│   └── NetworkUtils.cpp       # Network utilities implementation
│
├── data/                      # Data persistence layer
│   ├── tasks.db              # Task storage (SQLite format)
│   └── chatlog.txt           # Chat message history
│
├── LICENSE                    # MIT License
└── README.md                  # Project documentation
```

---

## ⚙️ Requirements

- **C++11 or later**
- **POSIX environment** (Linux / macOS)
- **GCC** (`g++`) or **Clang** (`clang++`)
- **pthread** library for multithreading
- **Network sockets** support

---

## 🚀 Quick Start

### Build the Project

```bash
# Clone the repository
git clone <repository-url>
cd Task-Manager-and-Real-Time-Chat-System

# Compile server
g++ -std=c++11 src/server.cpp src/TaskManager.cpp src/ChatManager.cpp src/Task.cpp src/Chat.cpp src/NetworkUtils.cpp -lpthread -o server

# Compile client
g++ -std=c++11 src/client.cpp src/Task.cpp src/Chat.cpp src/NetworkUtils.cpp -lpthread -o client
```

### Run the Application

1. **Start the Server:**
   ```bash
   ./server
   ```
   Server will start listening on port `8080`

2. **Connect Clients:**
   ```bash
   ./client
   ```
   Multiple clients can connect simultaneously

---

## 🎮 Usage

### Client Commands

- **Add a Task:**
  ```
  /task Complete project documentation
  ```

- **Send a Chat Message:**
  ```
  /chat Hello everyone!
  ```

### Server Behavior

- Accepts connections on `localhost:8080`
- Broadcasts all updates to connected clients
- Handles concurrent client connections with thread safety
- Logs activities for persistence

---

## 🏗️ Architecture

### Core Components

1. **Server (`server.cpp`)**
   - Socket server listening on port 8080
   - Multithreaded client handling
   - Message broadcasting to all clients
   - Thread-safe operations with mutex

2. **Client (`client.cpp`)**
   - TCP client connection
   - Command-line interface
   - Real-time message receiving
   - Command parsing (`/task`, `/chat`)

3. **Task Management**
   - `TaskManager`: Handles task operations
   - `Task`: Task entity with properties
   - Thread-safe task storage and retrieval

4. **Chat System**
   - `ChatManager`: Manages chat operations
   - `Chat`: Message entity handling
   - Real-time message broadcasting

### Design Patterns

- **Manager Pattern**: Separate managers for tasks and chat
- **Thread-Safe Singleton**: Shared resources with mutex protection
- **Observer Pattern**: Broadcast updates to all connected clients

---

## 🔧 Development

### Adding New Features

1. **Extend Task Properties**: Modify `Task.hpp` and `Task.cpp`
2. **Add New Commands**: Update client command parsing in `client.cpp`
3. **Enhance Persistence**: Extend data storage in the `data/` directory
4. **Network Protocols**: Modify `NetworkUtils.hpp` for new message types

### Testing

```bash
# Test with multiple clients
./server &
./client &
./client &
./client &
```

---

## 📝 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

**Copyright (c) 2025 Pranav Jagatprem Gawande**

---

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📞 Support

For questions or support, please open an issue in the repository.

---

**Built with ❤️ using C++ and Socket Programming**

