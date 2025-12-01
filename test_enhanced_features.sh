#!/bin/bash

# Enhanced Project Management System Test Script
echo "=== ENHANCED PROJECT MANAGEMENT SYSTEM TEST ==="
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

echo "🚀 Enhanced Features Ready to Test:"
echo
echo "🔒 1. PRIVACY UPDATE:"
echo "   - Private messages are NOT stored in chatlog.txt"
echo "   - Only public/group messages are logged"
echo
echo "👥 2. TASK ASSIGNMENT PERMISSIONS:"
echo "   - Only PM (pm1) and Admin (admin) can assign tasks"
echo "   - Developers and Testers cannot assign tasks"
echo
echo "📅 3. TASK DEADLINES:"
echo "   - Every task now has a deadline field"
echo "   - Default: 7 days, customizable with 'deadline:X'"
echo "   - Shows days until deadline and overdue status"
echo
echo "📊 4. DASHBOARD FEATURES:"
echo "   - /dashboard - Complete project overview"
echo "   - /overdue - Show overdue tasks"
echo "   - /recommend - Smart assignee recommendation"
echo
echo "⚖️ 5. SMART ASSIGNMENT:"
echo "   - Recommends assignee based on current workload"
echo "   - Considers user roles and active task count"
echo
echo "🎯 NEW COMMANDS TO TEST:"
echo "   /create Task Title | Description deadline:5"
echo "   /dashboard"
echo "   /recommend"
echo "   /overdue"
echo "   /pm username message (not logged to file)"
echo
echo "👤 TEST USERS & PERMISSIONS:"
echo "   admin/admin   - Can assign tasks ✅"
echo "   pm1/pm1       - Can assign tasks ✅"
echo "   dev1/dev1     - Cannot assign tasks ❌"
echo "   tester1/tester1 - Cannot assign tasks ❌"
echo
echo "🧪 TESTING SCENARIOS:"
echo "1. Login as admin, create tasks with deadlines"
echo "2. Try /assign as dev1 (should fail)"
echo "3. Try /assign as pm1 (should succeed)"
echo "4. Use /dashboard to see overview"
echo "5. Use /recommend for smart assignment"
echo "6. Send private messages (check they're not in chatlog.txt)"
echo
echo "▶️  START TESTING:"
echo "   Terminal 1: ./server"
echo "   Terminal 2: ./client"
echo "   Terminal 3: ./client"
echo
echo "✨ All enhanced features implemented and ready!"