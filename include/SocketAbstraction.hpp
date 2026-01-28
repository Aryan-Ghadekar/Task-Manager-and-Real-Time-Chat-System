#pragma once

#include <string>
#include <cstdint>

// Platform-specific includes and definitions
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    
    typedef SOCKET SocketHandle;
    #define INVALID_SOCKET_HANDLE INVALID_SOCKET
    #define SOCKET_ERROR_CODE WSAGetLastError()
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cerrno>
    #include <cstring>
    
    typedef int SocketHandle;
    #define INVALID_SOCKET_HANDLE -1
    #define SOCKET_ERROR_CODE errno
#endif

/**
 * Cross-platform socket abstraction layer
 * Provides unified interface for socket operations on Windows and POSIX systems
 */
class SocketAbstraction {
public:
    /**
     * Initialize socket subsystem (required on Windows, no-op on POSIX)
     * Must be called before any socket operations
     * @return true on success, false on failure
     */
    static bool initialize();
    
    /**
     * Cleanup socket subsystem (required on Windows, no-op on POSIX)
     * Should be called when done with all socket operations
     */
    static void cleanup();
    
    /**
     * Create a TCP socket
     * @return Socket handle or INVALID_SOCKET_HANDLE on failure
     */
    static SocketHandle createSocket();
    
    /**
     * Bind socket to address and port
     * @param socket Socket handle
     * @param port Port number to bind to
     * @param address IP address to bind to (default: any address)
     * @return true on success, false on failure
     */
    static bool bindSocket(SocketHandle socket, uint16_t port, const std::string& address = "0.0.0.0");
    
    /**
     * Listen for incoming connections
     * @param socket Socket handle
     * @param backlog Maximum queue length for pending connections
     * @return true on success, false on failure
     */
    static bool listenSocket(SocketHandle socket, int backlog = 10);
    
    /**
     * Accept incoming connection
     * @param socket Listening socket handle
     * @return New socket handle for the connection, or INVALID_SOCKET_HANDLE on failure
     */
    static SocketHandle acceptSocket(SocketHandle socket);
    
    /**
     * Connect to remote server
     * @param socket Socket handle
     * @param address Server IP address
     * @param port Server port number
     * @return true on success, false on failure
     */
    static bool connectSocket(SocketHandle socket, const std::string& address, uint16_t port);
    
    /**
     * Send data through socket
     * @param socket Socket handle
     * @param data Data buffer to send
     * @param length Length of data to send
     * @return Number of bytes sent, or -1 on error
     */
    static int sendData(SocketHandle socket, const char* data, size_t length);
    
    /**
     * Receive data from socket
     * @param socket Socket handle
     * @param buffer Buffer to store received data
     * @param length Maximum length to receive
     * @return Number of bytes received, 0 on connection close, or -1 on error
     */
    static int receiveData(SocketHandle socket, char* buffer, size_t length);
    
    /**
     * Close socket
     * @param socket Socket handle to close
     */
    static void closeSocket(SocketHandle socket);
    
    /**
     * Set socket option to reuse address
     * @param socket Socket handle
     * @return true on success, false on failure
     */
    static bool setReuseAddress(SocketHandle socket);
    
    /**
     * Get last socket error message
     * @return Error message string
     */
    static std::string getLastError();
    
    /**
     * Check if socket handle is valid
     * @param socket Socket handle to check
     * @return true if valid, false otherwise
     */
    static bool isValidSocket(SocketHandle socket);
};
