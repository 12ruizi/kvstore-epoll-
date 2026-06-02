#pragma once
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
  tcp_socket(int port = 2025);
  ~tcp_socket();
  tcp_socket(const tcp_socket &) = delete;
  tcp_socket &operator=(const tcp_socket &) = delete;
  int get_listen_fd() const;
  int Accept_tcp();
};