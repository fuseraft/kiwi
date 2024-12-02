#ifndef KIWI_NET_SOCKETMANAGER_H
#define KIWI_NET_SOCKETMANAGER_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <iostream>

#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/value.h"
#include <netinet/ip_icmp.h>

int x = SOCK_RAW;

class SocketManager {
 public:
  SocketManager();
  ~SocketManager();

  /**
   * Create a new socket.
   *
   * @param token A tracer token.
   * @param family The address family (e.g., AF_INET or AF_INET6).
   * @param type The socket type (e.g., SOCK_STREAM).
   * @param protocol The protocol number.
   * @return A unique socket ID.
   */
  k_value create(const Token& token,
                 const k_int& family = static_cast<k_int>(AF_INET),
                 const k_int& type = static_cast<k_int>(SOCK_STREAM),
                 const k_int& protocol = static_cast<k_int>(0));

  /**
   * Bind a socket to an address and port.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param address The IP address to bind to.
   * @param port The port number.
   */
  bool bind(const Token& token, const k_int& sock_id, const k_string& address,
            const k_int& port);

  /**
   * Listen for incoming connections.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param backlog The maximum number of queued connections.
   */
  bool listen(const Token& token, const k_int& sock_id,
              const k_int& backlog_value = static_cast<k_int>(5));

  /**
   * Accept an incoming connection.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param client_address Output parameter for the client's IP address.
   * @param client_port Output parameter for the client's port number.
   * @return A new socket ID for the accepted connection.
   */
  k_value accept(const Token& token, const k_int& sock_id,
                 k_string& client_address, k_int& client_port);

  /**
   * Connect a socket to a remote address.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param address The remote IP address.
   * @param port The remote port number.
   */
  bool connect(const Token& token, const k_int& sock_id,
               const k_string& address, const k_int& port);

  /**
   * Send data over a socket.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param data Pointer to the data buffer.
   * @return Number of bytes sent.
   */
  k_value send(const Token& token, const k_int& sock_id, k_value data);

  /**
   * Send data over a raw socket connection.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param destination The destination address.
   * @param data_value Pointer to the data buffer.
   * @return Number of bytes sent.
   */
  k_value sendRawPacket(const Token& token, const k_int& sock_id,
                        const k_string& destination, k_value data_value);

  /**
   * Receive data from a socket.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param length Maximum length of data to receive.
   * @return The received data as a string.
   */
  k_value receive(const Token& token, const k_int& sock_id,
                  const k_int& length);

  /**
   * Close a socket.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   */
  bool close(const Token& token, const k_int& sock_id);

  /**
   * Shut down a socket.
   *
   * @param token A tracer token.
   * @param sock_id The socket ID.
   * @param how How to shut down the socket (e.g., SHUT_RDWR).
   */
  bool shutdown(const Token& token, const k_int& sock_id,
                const k_int& how = static_cast<k_int>(SHUT_RDWR));

  /**
   * Check if a given string represents a valid IP address.
   *
   * @param input The input string.
   * @return Returns `true` if the given string represents a valid IP address.
   */
  bool isIPAddress(const k_string& input, int& addressFamily) {
    char buffer[INET6_ADDRSTRLEN];

    if (inet_pton(AF_INET, input.c_str(), buffer) == 1) {
      addressFamily = AF_INET;
      return true;
    }

    // Check if it's a valid IPv6 address
    if (inet_pton(AF_INET6, input.c_str(), buffer) == 1) {
      addressFamily = AF_INET6;
      return true;
    }

    // Not a valid IP address
    return false;
  }

  /**
   * Resolves a hostname to a list of IP addresses.
   *
   * @param input The hostname.
   * @return A list of IP addresses.
   */
  k_value resolveHostToIP(const k_string& hostname) {
    std::vector<k_value> ipAddresses;
    struct addrinfo hints, *res, *ptr;

    // Zero out the hints structure
    memset(&hints, 0, sizeof(hints));
    hints.ai_family =
        AF_UNSPEC;  // AF_INET for IPv4 only, AF_INET6 for IPv6 only
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (status != 0) {
      return std::make_shared<List>(ipAddresses);
    }

    for (ptr = res; ptr != nullptr; ptr = ptr->ai_next) {
      char ipStr[INET6_ADDRSTRLEN];

      if (ptr->ai_family == AF_INET) {  // IPv4
        struct sockaddr_in* ipv4 =
            reinterpret_cast<struct sockaddr_in*>(ptr->ai_addr);
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, sizeof(ipStr));
      } else if (ptr->ai_family == AF_INET6) {  // IPv6
        struct sockaddr_in6* ipv6 =
            reinterpret_cast<struct sockaddr_in6*>(ptr->ai_addr);
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipStr, sizeof(ipStr));
      } else {
        continue;
      }

      ipAddresses.emplace_back(ipStr);
    }

    freeaddrinfo(res);  // Free the linked list
    return std::make_shared<List>(ipAddresses);
  }

 private:
  int generate_socket_id();
  int get_socket(const Token& token, const k_int& sock_id);

  std::mutex mutex_;
  int next_socket_id_;
  std::unordered_map<int, int> sockets_;
  std::unordered_map<int, int> socket_families_;  // Store family per socket ID
};

SocketManager::SocketManager() : next_socket_id_(0) {}

SocketManager::~SocketManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto& pair : sockets_) {
    ::close(pair.second);
  }
  sockets_.clear();
  socket_families_.clear();
}

int SocketManager::generate_socket_id() {
  std::lock_guard<std::mutex> lock(mutex_);
  return next_socket_id_++;
}

int SocketManager::get_socket(const Token& token, const k_int& sock_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  int sock_id_value = static_cast<int>(sock_id);
  auto it = sockets_.find(sock_id_value);
  if (it == sockets_.end()) {
    throw SocketError(token,
                      "Socket ID not found: " + std::to_string(sock_id_value));
  }
  return it->second;
}

k_value SocketManager::create(const Token& token, const k_int& family_value,
                              const k_int& type_value,
                              const k_int& protocol_value) {
  int family = static_cast<int>(family_value);
  int type = static_cast<int>(type_value);
  int protocol = static_cast<int>(protocol_value);

  int sock = ::socket(family, type, protocol);
  if (sock == -1) {
    throw SocketError(
        token, "Failed to create socket: " + k_string(std::strerror(errno)));
  }

  int sock_id = generate_socket_id();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    sockets_[sock_id] = sock;
    socket_families_[sock_id] = family;
  }
  return static_cast<k_int>(sock_id);
}

bool SocketManager::bind(const Token& token, const k_int& sock_id,
                         const k_string& address, const k_int& port) {
  if (port < 0 || port > 65535) {
    throw SocketError(token, "Invalid port number: " + std::to_string(port));
  }

  int port_value = static_cast<int>(port);
  int sock = get_socket(token, sock_id);

  int family;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    family = socket_families_[sock_id];
  }

  struct addrinfo hints = {}, *res;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  k_string port_str = std::to_string(port_value);
  int ret = getaddrinfo(address.empty() ? nullptr : address.c_str(),
                        port_str.c_str(), &hints, &res);
  if (ret != 0) {
    throw SocketError(token,
                      "getaddrinfo failed: " + k_string(gai_strerror(ret)));
  }

  int bind_result = ::bind(sock, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);

  if (bind_result == -1) {
    throw SocketError(
        token, "Failed to bind socket: " + k_string(std::strerror(errno)));
  }

  return true;
}

bool SocketManager::listen(const Token& token, const k_int& sock_id,
                           const k_int& backlog_value) {
  int backlog = static_cast<int>(backlog_value);

  if (backlog <= 0) {
    throw SocketError(token,
                      "Backlog must be positive: " + std::to_string(backlog));
  }

  int sock = get_socket(token, sock_id);

  if (::listen(sock, backlog) == -1) {
    throw SocketError(
        token, "Failed to listen on socket: " + k_string(std::strerror(errno)));
  }

  return true;
}

k_value SocketManager::accept(const Token& token, const k_int& sock_id,
                              k_string& client_address, k_int& client_port) {
  int sock = get_socket(token, sock_id);

  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_sock;
  do {
    client_sock =
        ::accept(sock, (struct sockaddr*)&client_addr, &client_addr_len);
  } while (client_sock == -1 && errno == EINTR);

  if (client_sock == -1) {
    throw SocketError(token, "Failed to accept connection: " +
                                 k_string(std::strerror(errno)));
  }

  // RAII wrapper to ensure client_sock is closed if an exception is thrown
  struct ClientSocketGuard {
    int& sock_fd;
    ~ClientSocketGuard() {
      if (sock_fd != -1) {
        ::close(sock_fd);
      }
    }
  } client_sock_guard{client_sock};

  // Retrieve client address
  char addr_buffer[INET6_ADDRSTRLEN];
  void* addr_ptr;
  uint16_t port;

  if (client_addr.ss_family == AF_INET) {
    struct sockaddr_in* s = (struct sockaddr_in*)&client_addr;
    addr_ptr = &(s->sin_addr);
    port = ntohs(s->sin_port);
  } else if (client_addr.ss_family == AF_INET6) {
    struct sockaddr_in6* s = (struct sockaddr_in6*)&client_addr;
    addr_ptr = &(s->sin6_addr);
    port = ntohs(s->sin6_port);
  } else {
    throw SocketError(token, "Unknown address family.");
  }

  if (inet_ntop(client_addr.ss_family, addr_ptr, addr_buffer,
                sizeof(addr_buffer)) == nullptr) {
    throw SocketError(token, "Failed to get client address: " +
                                 k_string(std::strerror(errno)));
  }

  client_address = k_string(addr_buffer);
  client_port = static_cast<k_int>(port);

  // Generate new socket ID and store client socket
  int client_sock_id = generate_socket_id();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    sockets_[client_sock_id] = client_sock;
    socket_families_[client_sock_id] = client_addr.ss_family;
  }

  // Release ownership of client_sock
  client_sock_guard.sock_fd = -1;

  return static_cast<k_int>(client_sock_id);
}

bool SocketManager::connect(const Token& token, const k_int& sock_id,
                            const k_string& address, const k_int& port) {
  if (port < 0 || port > 65535) {
    throw SocketError(token, "Invalid port number: " + std::to_string(port));
  }

  int port_value = static_cast<int>(port);
  int sock = get_socket(token, sock_id);

  int family;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    family = socket_families_[sock_id];
  }

  struct addrinfo hints = {}, *res;
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;

  k_string port_str = std::to_string(port_value);
  int ret = getaddrinfo(address.c_str(), port_str.c_str(), &hints, &res);
  if (ret != 0) {
    throw SocketError(token,
                      "getaddrinfo failed: " + k_string(gai_strerror(ret)));
  }

  int connect_result = ::connect(sock, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);

  if (connect_result == -1) {
    throw SocketError(
        token, "Failed to connect socket: " + k_string(std::strerror(errno)));
  }

  return true;
}

k_value SocketManager::sendRawPacket(const Token& token, const k_int& sock_id,
                                     const k_string& destination,
                                     k_value data_value) {
  int sock = get_socket(token, sock_id);

  // Resolve destination address
  struct addrinfo hints = {}, *res;
  hints.ai_family = AF_UNSPEC;  // Support both IPv4 and IPv6
  hints.ai_socktype = SOCK_RAW;

  int ret = getaddrinfo(destination.c_str(), nullptr, &hints, &res);
  if (ret != 0) {
    throw SocketError(token, "Failed to resolve destination address: " +
                                 k_string(gai_strerror(ret)));
  }

  // Convert data to a raw byte buffer
  std::vector<char> raw_data;
  if (std::holds_alternative<k_string>(data_value)) {
    raw_data.assign(std::get<k_string>(data_value).begin(),
                    std::get<k_string>(data_value).end());
  } else if (std::holds_alternative<k_list>(data_value)) {
    k_list data_list = std::get<k_list>(data_value);
    for (const auto& elem : data_list->elements) {
      if (std::holds_alternative<k_int>(elem)) {
        raw_data.push_back(static_cast<char>(std::get<k_int>(elem)));
      }
    }
  }

  if (raw_data.empty()) {
    freeaddrinfo(res);
    return static_cast<k_int>(0);  // Nothing to send
  }

  // Iterate over all resolved addresses and send data
  ssize_t sent_bytes = -1;
  for (struct addrinfo* ptr = res; ptr != nullptr; ptr = ptr->ai_next) {
    sent_bytes = ::sendto(sock, raw_data.data(), raw_data.size(), 0,
                          ptr->ai_addr, ptr->ai_addrlen);
    if (sent_bytes != -1) {
      break;  // Successfully sent the packet
    }
  }

  freeaddrinfo(res);

  if (sent_bytes == -1) {
    throw SocketError(
        token, "Failed to send raw packet: " + k_string(std::strerror(errno)));
  }

  return static_cast<k_int>(sent_bytes);
}

k_value SocketManager::send(const Token& token, const k_int& sock_id,
                            k_value data_value) {
  int sock = get_socket(token, sock_id);

  k_string data;
  if (std::holds_alternative<k_string>(data_value)) {
    data = std::get<k_string>(data_value);
  } else if (std::holds_alternative<k_list>(data_value)) {
    // Assuming k_list of k_int (bytes)
    k_list data_list = std::get<k_list>(data_value);
    data.reserve(data_list->elements.size());
    for (const auto& elem : data_list->elements) {
      if (std::holds_alternative<k_int>(elem)) {
        k_int byte_value = std::get<k_int>(elem);
        if (byte_value < 0 || byte_value > 255) {
          throw SocketError(token, "Byte values must be between 0 and 255");
        }
        data += static_cast<char>(byte_value);
      } else {
        throw SocketError(token, "Data list must contain integers");
      }
    }
  } else {
    throw SocketError(token, "Data must be a string or list of integers");
  }

  const char* data_ptr = data.data();
  size_t data_size = data.size();

  if (data_size == 0) {
    // Nothing to send.
    return static_cast<k_int>(0);
  }

  ssize_t total_bytes_sent = 0;

  while (static_cast<size_t>(total_bytes_sent) < data_size) {
    ssize_t bytes_sent = ::send(sock, data_ptr + total_bytes_sent,
                                data_size - total_bytes_sent, 0);
    if (bytes_sent == -1) {
      if (errno == EINTR) {
        continue;  // Retry if interrupted
      } else {
        throw SocketError(
            token, "Failed to send data: " + k_string(std::strerror(errno)));
      }
    }
    total_bytes_sent += bytes_sent;
  }

  return static_cast<k_int>(total_bytes_sent);
}

k_value SocketManager::receive(const Token& token, const k_int& sock_id,
                               const k_int& length_value) {
  if (length_value <= 0) {
    throw SocketError(
        token, "Length must be positive: " + std::to_string(length_value));
  }

  size_t length = static_cast<size_t>(length_value);
  int sock = get_socket(token, sock_id);
  std::vector<char> buffer(length);

  ssize_t bytes_received;
  while (true) {
    bytes_received = ::recv(sock, buffer.data(), length, 0);
    if (bytes_received == -1) {
      if (errno == EINTR) {
        continue;  // Retry if interrupted
      } else {
        throw SocketError(
            token, "Failed to receive data: " + k_string(std::strerror(errno)));
      }
    }
    break;
  }

  if (bytes_received == 0) {
    // Peer has performed an orderly shutdown
    return k_string();  // Return an empty string
  }

  // Convert received data to a string
  k_string data_str(buffer.begin(), buffer.begin() + bytes_received);
  return data_str;
}

bool SocketManager::close(const Token& token, const k_int& sock_id) {
  int sock_id_value = static_cast<int>(sock_id);
  int sock;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sockets_.find(sock_id_value);
    if (it == sockets_.end()) {
      throw SocketError(
          token, "Socket ID not found: " + std::to_string(sock_id_value));
    }
    sock = it->second;
    sockets_.erase(it);
    socket_families_.erase(sock_id_value);
  }

  if (::close(sock) == -1) {
    throw SocketError(
        token, "Failed to close socket: " + k_string(std::strerror(errno)));
  }

  return true;
}

bool SocketManager::shutdown(const Token& token, const k_int& sock_id,
                             const k_int& how_value) {
  int how = static_cast<int>(how_value);
  if (how != SHUT_RD && how != SHUT_WR && how != SHUT_RDWR) {
    throw SocketError(token, "Invalid shutdown mode: " + std::to_string(how));
  }

  int sock = get_socket(token, sock_id);

  if (::shutdown(sock, how) == -1) {
    throw SocketError(
        token, "Failed to shutdown socket: " + k_string(std::strerror(errno)));
  }

  return true;
}

#endif
