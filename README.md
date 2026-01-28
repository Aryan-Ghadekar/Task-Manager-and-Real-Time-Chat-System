# Task Manager & Real-Time Chat System - Complete Guide

## 🎉 Overview

A **cross-platform** (macOS + Windows + Linux) task management and real-time chat system with:
- **Backend**: C++ server with TCP sockets + HTTP/REST API
- **Frontend**: Electron + React with modern dark theme UI
- **Features**: Task management, real-time chat, user authentication, role-based access

---

## 🚀 Quick Start

### Prerequisites

**macOS/Linux:**
- C++ compiler (g++/clang)
- Node.js 18+ and npm
- Make (optional, for Makefile builds)

**Windows:**
- Visual Studio 2019+ OR MinGW-w64
- CMake
- Node.js 18+ and npm

### Installation

```bash
# Clone or navigate to project directory
cd Task-Manager-and-Real-Time-Chat-System

# Install frontend dependencies
cd frontend
npm install
cd ..
```

---

## 🏃 Running the Application

### Step 1: Start the Backend Server

The backend runs **two servers** simultaneously:
- **TCP Socket Server** (port 8080) - for CLI clients
- **HTTP API Server** (port 8081) - for frontend

**Option A: Using Makefile (macOS/Linux)**

```bash
# Build the server with API support
make clean
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude \
    src/server.cpp src/TaskManager.cpp src/ChatManager.cpp \
    src/Task.cpp src/Chat.cpp src/User.cpp \
    src/NetworkUtils.cpp src/SocketAbstraction.cpp \
    src/HTTPServer.cpp -o server_api

# Run the server
./server_api
```

**Option B: Using CMake (All Platforms)**

```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run the server
./server_api
```

You should see:
```
=== JIRA-like Task Manager - Dual Server Mode ===
TCP Socket Server: port 8080 (for CLI clients)
HTTP API Server: port 8081 (for frontend)
Available users: admin, pm1, dev1, tester1
=================================================
Starting HTTP API server on port 8081...
TCP Socket Server running on port 8080...
```

### Step 2: Start the Frontend

**In a new terminal:**

```bash
cd frontend

# Development mode (with hot reload)
npm run dev

# The app will open at http://localhost:5173
```

**Or run as Electron app:**

```bash
# Install Electron dependencies first
npm install

# Run in Electron
NODE_ENV=development npm run electron:dev
```

---

## 🎨 Using the Application

### Login

Use one of the demo accounts:
- **admin** / admin (Admin role)
- **pm1** / pm1 (Project Manager)
- **dev1** / dev1 (Developer)
- **tester1** / tester1 (Tester)

### Dashboard Features

**Tasks Panel (Left)**
- View all tasks, your tasks, or overdue tasks
- Create new tasks with title, description, and deadline
- Update task status (To Do → In Progress → In Review → Done)
- See priority indicators and deadline tracking
- Real-time updates every 5 seconds

**Chat Panel (Right)**
- Send and receive messages in real-time
- Auto-refresh every 3 seconds
- See message timestamps and sender names
- Scroll to latest messages automatically

---

## 📁 Project Structure

```
Task-Manager-and-Real-Time-Chat-System/
├── include/                    # C++ headers
│   ├── SocketAbstraction.hpp  # Cross-platform sockets
│   ├── HTTPServer.hpp          # REST API server
│   ├── TaskManager.hpp
│   ├── ChatManager.hpp
│   ├── Task.hpp
│   ├── Chat.hpp
│   ├── User.hpp
│   └── NetworkUtils.hpp
├── src/                        # C++ implementation
│   ├── SocketAbstraction.cpp
│   ├── HTTPServer.cpp
│   ├── server.cpp              # Original TCP server
│   ├── server_with_api.cpp    # Dual server (TCP + HTTP)
│   ├── client.cpp              # CLI client
│   ├── TaskManager.cpp
│   ├── ChatManager.cpp
│   └── ...
├── frontend/                   # Electron + React app
│   ├── src/
│   │   ├── components/
│   │   │   ├── Login.jsx
│   │   │   ├── Dashboard.jsx
│   │   │   ├── TaskList.jsx
│   │   │   ├── Chat.jsx
│   │   │   └── CreateTask.jsx
│   │   ├── App.jsx
│   │   ├── api.js              # API client
│   │   └── index.css           # Design system
│   ├── electron/
│   │   └── main.js             # Electron main process
│   ├── package.json
│   └── vite.config.js
├── data/                       # Data storage
│   ├── tasks.db
│   └── chatlog.txt
├── CMakeLists.txt              # Cross-platform build
├── Makefile                    # macOS/Linux build
├── BUILD.md                    # Build instructions
└── README.md                   # This file
```

---

## 🔌 API Endpoints

The HTTP API server (port 8081) provides:

### Authentication
- `POST /api/login` - User login
- `POST /api/logout` - User logout

### Tasks
- `GET /api/tasks` - Get all tasks
- `GET /api/tasks/my` - Get my tasks
- `GET /api/tasks/overdue` - Get overdue tasks
- `POST /api/tasks` - Create new task
- `PUT /api/tasks/:id/status` - Update task status

### Chat
- `GET /api/chat` - Get recent messages
- `POST /api/chat` - Send message

### Users
- `GET /api/users/online` - Get online users
- `GET /api/dashboard` - Get dashboard data

All endpoints (except login) require `Authorization: Bearer <token>` header.

---

## 🛠️ Development

### Backend Development

```bash
# Build with debug symbols
make debug

# Or with CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Frontend Development

```bash
cd frontend

# Start dev server with hot reload
npm run dev

# Build for production
npm run build

# Build Electron app
npm run electron:build
```

---

## 🧪 Testing

### Test Backend

```bash
# Start server
./server_api

# In another terminal, use CLI client
./client

# Login and test commands
/login admin admin
/create Test Task | Testing the system
/list
/chat Hello from CLI!
```

### Test Frontend

1. Start backend server
2. Start frontend: `cd frontend && npm run dev`
3. Open http://localhost:5173
4. Login with demo account
5. Create tasks, send messages
6. Open multiple browser tabs to test real-time updates

---

## 📦 Building for Production

### Backend

```bash
# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Frontend (Electron App)

```bash
cd frontend
npm run electron:build

# Output in frontend/dist-electron/
# - macOS: .dmg file
# - Windows: .exe installer
```

---

## 🎯 Features

### ✅ Implemented
- Cross-platform backend (macOS, Windows, Linux)
- Socket abstraction layer
- HTTP/REST API
- User authentication with sessions
- Task management (CRUD operations)
- Real-time chat
- Modern React UI with dark theme
- Electron desktop app
- Auto-refresh for tasks and messages
- Priority indicators
- Deadline tracking
- Status management

### 🚧 Future Enhancements
- WebSocket for true real-time updates
- Task assignments to specific users
- File attachments
- Notifications
- Search and filtering
- User profiles
- Task comments
- Analytics dashboard

---

## 🐛 Troubleshooting

### Backend won't start
- Check if ports 8080 and 8081 are available
- Ensure `httplib.h` is in `include/` directory
- Verify all source files are compiled

### Frontend can't connect
- Ensure backend is running on port 8081
- Check browser console for CORS errors
- Verify API_BASE_URL in `frontend/src/api.js`

### Build errors
```bash
# Clean and rebuild
make clean && make

# Or with CMake
rm -rf build && mkdir build && cd build && cmake .. && make
```

---

## 📄 License

ISC

---

## 👥 Demo Accounts

| Username | Password | Role |
|----------|----------|------|
| admin | admin | Admin |
| pm1 | pm1 | Project Manager |
| dev1 | dev1 | Developer |
| tester1 | tester1 | Tester |

---

## 🎨 UI Preview

The frontend features:
- **Modern dark theme** with gradient accents
- **Smooth animations** and transitions
- **Responsive layout** (desktop-optimized)
- **Real-time updates** for tasks and chat
- **Intuitive task management** with drag-free status updates
- **Clean, professional design** inspired by modern SaaS apps

---

**Enjoy your cross-platform task management system! 🚀**
