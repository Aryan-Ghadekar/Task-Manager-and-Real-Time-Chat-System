#include "../include/SocketAbstraction.hpp"
#include <iostream>
#include <sstream>

bool SocketAbstraction::initialize() {
#ifdef _WIN32
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (result != 0) {
    std::cerr << "WSAStartup failed with error: " << result << std::endl;
    return false;
  }
  return true;
#else
  // No initialization needed on POSIX systems
  return true;
#endif
}

void SocketAbstraction::cleanup() {
#ifdef _WIN32
  WSACleanup();
#else
  // No cleanup needed on POSIX systems
#endif
}

SocketHandle SocketAbstraction::createSocket() {
  SocketHandle sock = socket(AF_INET, SOCK_STREAM, 0);
  if (!isValidSocket(sock)) {
    std::cerr << "Failed to create socket: " << getLastError() << std::endl;
    return INVALID_SOCKET_HANDLE;
  }
  return sock;
}

bool SocketAbstraction::bindSocket(SocketHandle socket, uint16_t port,
                                   const std::string &address) {
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (address == "0.0.0.0" || address.empty()) {
    serverAddr.sin_addr.s_addr = INADDR_ANY;
  } else {
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
      std::cerr << "Invalid address: " << address << std::endl;
      return false;
    }
  }

  if (bind(socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Bind failed: " << getLastError() << std::endl;
    return false;
  }

  return true;
}

bool SocketAbstraction::listenSocket(SocketHandle socket, int backlog) {
  if (listen(socket, backlog) < 0) {
    std::cerr << "Listen failed: " << getLastError() << std::endl;
    return false;
  }
  return true;
}

SocketHandle SocketAbstraction::acceptSocket(SocketHandle socket) {
  SocketHandle clientSocket = accept(socket, nullptr, nullptr);
  if (!isValidSocket(clientSocket)) {
    std::cerr << "Accept failed: " << getLastError() << std::endl;
    return INVALID_SOCKET_HANDLE;
  }
  return clientSocket;
}

bool SocketAbstraction::connectSocket(SocketHandle socket,
                                      const std::string &address,
                                      uint16_t port) {
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Invalid address: " << address << std::endl;
    return false;
  }

  if (connect(socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Connect failed: " << getLastError() << std::endl;
    return false;
  }

  return true;
}

int SocketAbstraction::sendData(SocketHandle socket, const char *data,
                                size_t length) {
#ifdef _WIN32
  // Windows doesn't have MSG_NOSIGNAL, but send() doesn't raise SIGPIPE anyway
  int result = send(socket, data, static_cast<int>(length), 0);
#else
  // POSIX: use MSG_NOSIGNAL to prevent SIGPIPE
  int result = send(socket, data, length, MSG_NOSIGNAL);
#endif

  if (result < 0) {
    std::cerr << "Send failed: " << getLastError() << std::endl;
  }

  return result;
}

int SocketAbstraction::receiveData(SocketHandle socket, char *buffer,
                                   size_t length) {
#ifdef _WIN32
  int result = recv(socket, buffer, static_cast<int>(length), 0);
#else
  int result = recv(socket, buffer, length, 0);
#endif

  if (result < 0) {
    std::cerr << "Receive failed: " << getLastError() << std::endl;
  }

  return result;
}

void SocketAbstraction::closeSocket(SocketHandle socket) {
  if (!isValidSocket(socket)) {
    return;
  }

#ifdef _WIN32
  closesocket(socket);
#else
  close(socket);
#endif
}

bool SocketAbstraction::setReuseAddress(SocketHandle socket) {
  int opt = 1;
#ifdef _WIN32
  if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
                 sizeof(opt)) < 0) {
#else
  if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
    std::cerr << "Failed to set SO_REUSEADDR: " << getLastError() << std::endl;
    return false;
  }
  return true;
}

std::string SocketAbstraction::getLastError() {
#ifdef _WIN32
  int errorCode = WSAGetLastError();
  std::ostringstream oss;
  oss << "Error code: " << errorCode;

  // Common Windows socket errors
  switch (errorCode) {
  case WSAEACCES:
    return "Permission denied";
  case WSAEADDRINUSE:
    return "Address already in use";
  case WSAEADDRNOTAVAIL:
    return "Address not available";
  case WSAECONNREFUSED:
    return "Connection refused";
  case WSAECONNRESET:
    return "Connection reset by peer";
  case WSAEHOSTUNREACH:
    return "Host unreachable";
  case WSAENETUNREACH:
    return "Network unreachable";
  case WSAETIMEDOUT:
    return "Connection timed out";
  case WSAEWOULDBLOCK:
    return "Operation would block";
  default:
    return oss.str();
  }
#else
  return std::string(strerror(errno));
#endif
}

bool SocketAbstraction::isValidSocket(SocketHandle socket) {
#ifdef _WIN32
  return socket != INVALID_SOCKET;
#else
  return socket >= 0;
#endif
}
