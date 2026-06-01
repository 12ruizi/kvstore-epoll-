#pragma once
#include "../utils/memory.h"
#include "connect_info/connect_info.h"
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

class tcp_socket {
private:
  int _port;
  int _socket_fd;
  int create();

public:
  Conn_pool _conn_pool;
  tcp_socket(int port = 2025, size_t conn_blocks = 1024,
             size_t block_size = sizeof(ConnectionInfo));
  ~tcp_socket();
  tcp_socket(const tcp_socket &) = delete;
  tcp_socket &operator=(const tcp_socket &) = delete;
  int fd() const;
  int Accept_tcp();
};