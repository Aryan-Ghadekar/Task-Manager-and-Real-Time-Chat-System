#!/bin/bash

# JIRA-like Task Manager Test Script
echo "=== JIRA-like Task Manager Test Script ==="
echo "This script will build and test the system components"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ $2${NC}"
    else
        echo -e "${RED}✗ $2${NC}"
    fi
}

print_info() {
    echo -e "${YELLOW}ℹ $1${NC}"
}

# Clean previous builds
print_info "Cleaning previous builds..."
make clean > /dev/null 2>&1

# Build the project
print_info "Building the project..."
make all > build.log 2>&1
build_status=$?
print_status $build_status "Project build"

if [ $build_status -ne 0 ]; then
    echo "Build failed. Check build.log for details."
    cat build.log
    exit 1
fi

# Check if executables were created
if [ -f "server" ] && [ -f "client" ]; then
    print_status 0 "Executables created successfully"
else
    print_status 1 "Executables not found"
    exit 1
fi

# Check data directory
if [ -d "data" ]; then
    print_status 0 "Data directory exists"
else
    print_status 1 "Data directory missing"
fi

# Test demo compilation
print_info "Testing demo compilation..."
g++ -std=c++11 -Iinclude demo.cpp src/TaskManager.cpp src/ChatManager.cpp src/Task.cpp src/Chat.cpp src/User.cpp src/NetworkUtils.cpp -pthread -o demo > demo_build.log 2>&1
demo_status=$?
print_status $demo_status "Demo compilation"

if [ $demo_status -eq 0 ]; then
    print_info "Running demo..."
    ./demo
    rm -f demo
fi

# Test server startup (brief test)
print_info "Testing server startup..."
timeout 3s ./server > server_test.log 2>&1 &
server_pid=$!
sleep 1

if kill -0 $server_pid 2>/dev/null; then
    print_status 0 "Server starts successfully"
    kill $server_pid 2>/dev/null
else
    print_status 1 "Server startup failed"
    cat server_test.log
fi

# Check file structure
print_info "Checking project structure..."
required_files=(
    "include/User.hpp"
    "include/Task.hpp" 
    "include/Chat.hpp"
    "include/TaskManager.hpp"
    "include/ChatManager.hpp"
    "include/NetworkUtils.hpp"
    "src/server.cpp"
    "src/client.cpp"
    "src/User.cpp"
    "src/Task.cpp"
    "src/Chat.cpp"
    "src/TaskManager.cpp"
    "src/ChatManager.cpp"
    "src/NetworkUtils.cpp"
    "Makefile"
    "README.md"
    "USAGE.md"
)

missing_files=0
for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo -e "  ${GREEN}✓${NC} $file"
    else
        echo -e "  ${RED}✗${NC} $file"
        ((missing_files++))
    fi
done

if [ $missing_files -eq 0 ]; then
    print_status 0 "All required files present"
else
    print_status 1 "$missing_files files missing"
fi

# Test summary
echo
echo "=== Test Summary ==="
echo "Build Status: $([ $build_status -eq 0 ] && echo "PASS" || echo "FAIL")"
echo "Demo Status: $([ $demo_status -eq 0 ] && echo "PASS" || echo "FAIL")"
echo "File Structure: $([ $missing_files -eq 0 ] && echo "PASS" || echo "FAIL")"

echo
echo "=== Next Steps ==="
echo "1. Start server: ./server"
echo "2. Connect clients: ./client"
echo "3. Login with: admin/admin, pm1/pm1, dev1/dev1, tester1/tester1"
echo "4. Use /help for available commands"
echo "5. Check USAGE.md for detailed instructions"

# Cleanup
rm -f build.log demo_build.log server_test.log

echo
print_info "Test script completed!"