cmake_minimum_required(VERSION 3.15)
project(TaskManagerChatSystem VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    message(STATUS "Building for Windows")
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
elseif(APPLE)
    message(STATUS "Building for macOS")
endif()

# Compiler-specific flags
if(MSVC)
    # MSVC compiler flags
    add_compile_options(/W4 /WX-)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    # GCC/Clang compiler flags
    add_compile_options(-Wall -Wextra)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(COMMON_SOURCES
    src/Task.cpp
    src/Chat.cpp
    src/User.cpp
    src/NetworkUtils.cpp
    src/SocketAbstraction.cpp
)

set(SERVER_SOURCES
    ${COMMON_SOURCES}
    src/TaskManager.cpp
    src/ChatManager.cpp
    src/server.cpp
)

set(CLIENT_SOURCES
    ${COMMON_SOURCES}
    src/client.cpp
)

# Server executable
add_executable(server ${SERVER_SOURCES})

# Client executable
add_executable(client ${CLIENT_SOURCES})

# Platform-specific linking
if(WIN32)
    # Link Windows socket library
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
else()
    # Link pthread on POSIX systems
    find_package(Threads REQUIRED)
    target_link_libraries(server Threads::Threads)
    target_link_libraries(client Threads::Threads)
endif()

# Create data directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)

# Copy data directory to build directory (for existing data files)
if(EXISTS ${CMAKE_SOURCE_DIR}/data)
    file(COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)
endif()

# Installation rules
install(TARGETS server client
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/data/
    DESTINATION share/task-manager/data
    OPTIONAL
)

# Debug and Release configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
