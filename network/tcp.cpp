#include "tcp.h"
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

tcp_socket::tcp_socket(int port) : _port(port), _socket_fd(-1) {
  if (create() < 0) {
    throw std::runtime_error("tcp socket create failed");
  }
}

tcp_socket::~tcp_socket() {
  if (_socket_fd >= 0) {
    close(_socket_fd);
  }
}

int tcp_socket::create() {
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
  int opt = 1;
  ret = setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (ret < 0) {
    perror("setsockopt");
    return ret;
  }
  ret = bind(_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    perror("bind");
    return ret;
  }

  ret = listen(_socket_fd, 1);
  if (ret < 0) {
    perror("listen");
    return -1;
  }
  return _socket_fd;
}

int tcp_socket::get_listen_fd() const {
  if (_socket_fd < 0) {
    throw std::runtime_error("tcp socket fd is invalid");
  }
  return _socket_fd;
}

int tcp_socket::Accept_tcp() {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int conn_fd = accept(get_listen_fd(), (struct sockaddr *)&addr, &len);
  if (conn_fd < 0) {
    perror("Accept_tcp");
    return -1;
  }
  return conn_fd;
}