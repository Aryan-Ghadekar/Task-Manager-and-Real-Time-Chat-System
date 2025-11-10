#!/bin/bash

# Test script for enhanced JIRA-like Task Manager
echo "=== Testing Enhanced JIRA-like Task Manager ==="
echo

# Check if executables exist
if [ ! -f "./server" ] || [ ! -f "./client" ]; then
    echo "❌ Executables not found. Building project..."
    make clean && make all
    if [ $? -ne 0 ]; then
        echo "❌ Build failed!"
        exit 1
    fi
    echo "✅ Build successful!"
fi

# Check data directory
if [ ! -d "./data" ]; then
    echo "📁 Creating data directory..."
    mkdir -p data
    touch data/tasks.db data/chatlog.txt
fi

echo "✅ Project structure verified"
echo
echo "🚀 Ready to test! Start the server and clients:"
echo "   Terminal 1: ./server"
echo "   Terminal 2: ./client"
echo "   Terminal 3: ./client"
echo
echo "📋 Test the following features:"
echo "   1. Login with: admin/admin, pm1/pm1, dev1/dev1, tester1/tester1"
echo "   2. Create tasks: /create Fix bug | Description here"
echo "   3. View online users: /online"
echo "   4. Send private messages: /pm 2 Hello there"
echo "   5. Assign tasks: /assign 1 2"
echo "   6. Update status: /status 1 PROGRESS"
echo "   7. Set priority: /priority 1 HIGH"
echo "   8. Add comments: /comment 1 Working on this"
echo "   9. List tasks: /list"
echo "   10. View my tasks: /mytasks"
echo "   11. Public chat: /chat Hello everyone"
echo
echo "🔍 Error handling tests:"
echo "   - Try invalid commands"
echo "   - Use wrong task IDs"
echo "   - Send empty messages"
echo "   - Test with invalid user IDs"
echo
echo "✨ All features enhanced with robust error handling!"