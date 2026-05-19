#ifndef CONNECT_INFO_H
#define CONNECT_INFO_H
#include "protocol_parser.h"
#include "ring_buffer.h"
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
//抽象的请求数据结构，
// class Request_base {
// public:
//   std::string type;
//   char *data;         //数据初始指针位置
//   std::string method; //请求方法
//   virtual ~Request_base() = default;
// };
// class Request_http : public Request_base {
// public:
//   std::string url;     //请求url
//   std::string headers; //请求头
// };
// class Request_kv : public Request_base {
// public:
//   std::string key;
//   std::string value;
// };
// class Request_factory {
// public:
//   static std::unique_ptr<Request_base> create(const std::string &type) {
//     if (type == "http") {
//       return std::make_unique<Request_http>();
//     } else if (type == "kv") {
//       return std::make_unique<Request_kv>();
//     } else {
//       throw std::runtime_error("unknown request type");
//     }
//   }
// };

class ConnectionInfo {
public:
  int fd;                   // 套接字描述符
  struct sockaddr_in addr;  // 客户端地址信息
  ring_buffer read_buffer;  // 读缓冲区
  ring_buffer write_buffer; // 写缓冲区
  ProtocolParser *_Parser;  //协议解析指针
  //提供重置方法，用于重用连接信息
  void reset() {
    fd = -1;
    addr = {};
    _Parser = nullptr;
  }
  //初始化的时候需要设置协议的类型
  void set_parser(ProtocolParser *parser) { _Parser = parser; }
  ConnectionInfo() : fd(-1), _Parser(nullptr) {}
  ~ConnectionInfo() {
    if (fd >= 0) {
      close(fd);
    }
    if (_Parser) {
      delete _Parser;
    }
  }
};

#endif
