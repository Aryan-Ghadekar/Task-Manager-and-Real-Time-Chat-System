# JIRA-like Task Manager Usage Guide

## Quick Start

### 1. Build the Project
```bash
make all
```

### 2. Start the Server
```bash
make run-server
# or
./server
```

### 3. Connect Clients
```bash
make run-client
# or
./client
```

## Available Users
- **admin** (Admin) - Full permissions
- **pm1** (Project Manager) - Task management permissions
- **dev1** (Developer) - Development permissions  
- **tester1** (Tester) - Testing permissions

*Password: same as username*

## Commands Reference

### Authentication
```bash
/login <username> <password>
```

### Task Management
```bash
# Create a new task
/create Fix login bug | Resolve authentication issues

# Assign task to user
/assign 1 3

# Update task status
/status 1 PROGRESS    # TODO, PROGRESS, REVIEW, DONE, BLOCKED

# Set task priority
/priority 1 HIGH      # LOW, MEDIUM, HIGH, CRITICAL

# Add comment to task
/comment 1 Working on this issue

# List all tasks
/list

# Show my assigned tasks
/mytasks
```

### Chat System
```bash
# Send public message
/chat Hello team!

# Send private message
/pm 2 Can you review my code?

# Task-related discussions are automatically linked
```

### System Commands
```bash
# Show help
/help

# Exit application
/quit
```

## Example Workflow

1. **Login as Project Manager:**
   ```bash
   /login pm1 pm1
   ```

2. **Create Tasks:**
   ```bash
   /create Implement user authentication | Add login/logout functionality
   /create Fix database connection | Resolve connection timeout issues
   ```

3. **Assign Tasks:**
   ```bash
   /assign 1 3  # Assign task 1 to dev1 (userId 3)
   /assign 2 4  # Assign task 2 to tester1 (userId 4)
   ```

4. **Team Communication:**
   ```bash
   /chat Tasks have been assigned, please check your assignments
   /pm 3 Please prioritize the authentication task
   ```

5. **Update Task Progress:**
   ```bash
   /status 1 PROGRESS
   /priority 1 HIGH
   /comment 1 Started working on authentication module
   ```

## Real-time Features

- **Live Updates:** All connected users receive real-time notifications
- **Task Changes:** Status updates, assignments, and comments are broadcast
- **Chat Messages:** Instant messaging with public and private options
- **User Status:** Online/offline status tracking
- **System Notifications:** Join/leave notifications

## Data Persistence

- **Tasks:** Stored in `data/tasks.db`
- **Chat History:** Logged in `data/chatlog.txt`
- **User Sessions:** Maintained in memory during runtime

## Architecture Highlights

- **Multithreaded Server:** Each client handled by separate thread
- **Thread Safety:** Mutex protection for shared resources
- **Role-based Permissions:** Different access levels for users
- **Modular Design:** Separate managers for tasks and chat
- **Real-time Broadcasting:** Instant updates to all connected clients