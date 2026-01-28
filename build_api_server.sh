#!/bin/bash

# Build script for Task Manager with HTTP API

echo "Building Task Manager with HTTP API support..."

# Compile all source files except the main files
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/TaskManager.cpp -o obj/TaskManager.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/ChatManager.cpp -o obj/ChatManager.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/Task.cpp -o obj/Task.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/Chat.cpp -o obj/Chat.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/User.cpp -o obj/User.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/NetworkUtils.cpp -o obj/NetworkUtils.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/SocketAbstraction.cpp -o obj/SocketAbstraction.o
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude -c src/HTTPServer.cpp -o obj/HTTPServer.o

# Compile server with all command processing
g++ -std=c++11 -Wall -Wextra -pthread -Iinclude \
    src/server.cpp src/HTTPServer.cpp \
    obj/TaskManager.o obj/ChatManager.o obj/Task.o obj/Chat.o obj/User.o \
    obj/NetworkUtils.o obj/SocketAbstraction.o \
    -o server_api

echo "Build complete! Run with: ./server_api"
