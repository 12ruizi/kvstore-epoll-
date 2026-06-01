#ifndef CONNECT_INFO_H
#define CONNECT_INFO_H
#include "../network/protocol_parser/protocol_parser.h"
#include "Request.h"
#include "ring_buffer.h"
#include <arpa/inet.h>
#include <atomic>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
class ConnectionInfo {
public:
  int fd;                   // 套接字描述符
  struct sockaddr_in addr;  // 客户端地址信息
  ring_buffer read_buffer;  // 读缓冲区
  ring_buffer write_buffer; // 写缓冲区
  std::unique_ptr<REquest> _request;
  //提供重置方法，用于重用连接信息
  void reset() {
    fd = -1;
    addr = {};

    _request.reset();
  }
  ConnectionInfo() : fd(-1) {}
  ~ConnectionInfo() {
    if (fd >= 0) {
      close(fd);
    }
    _request.reset();
  }
};

#endif