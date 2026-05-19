#pragma once
#include "connect_info.h"
#include "memory.h"
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
class tcp_socket {
private:
  int _port;
  int _socket_fd;
  int create() {
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0) {
      perror("socket");
      return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);
    int ret = -1;
    ret = bind(_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
      perror("bind");
      return ret;
    }
    int opt = 1;
    ret = setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret < 0) {
      perror("setsockopt");
      return ret;
    }
    ret = listen(_socket_fd, 1);
    if (ret < 0) {
      perror("listen");
      return -1;
    }
    return _socket_fd;
  };

public:
  Conn_pool _conn_pool;
  tcp_socket(int port = 2025, size_t conn_blocks = 1024,
             size_t block_size = sizeof(ConnectionInfo))
      : _port(port), _socket_fd(-1), _conn_pool(conn_blocks, block_size) {
    if (create() < 0) {
      throw std::runtime_error("tcp socket create failed");
    }
  };
  ~tcp_socket() {
    if (_socket_fd >= 0) {
      close(_socket_fd);
    }
  }
  tcp_socket(const tcp_socket &) = delete;
  tcp_socket &operator=(const tcp_socket &) = delete;
  int fd() const {
    if (_socket_fd < 0) {
      throw std::runtime_error("tcp socket fd is invalid");
    }
    return _socket_fd;
  }
  int Accept_tcp() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int conn_fd = accept(fd(), (struct sockaddr *)&addr, &len);
    if (conn_fd < 0) {
      perror("Accept_tcp");
      return -1;
    }
    auto conn_info = _conn_pool.acquire();
    conn_info->fd = conn_fd;
    conn_info->addr = addr;
    _conn_pool.add_map(conn_fd, conn_info);
    return conn_info->fd;
  }
};
