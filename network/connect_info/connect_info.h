#ifndef CONNECT_INFO_H
#define CONNECT_INFO_H
#include "../network/protocol_parser/protocol_parser.h"
#include "../task_and_router/Router.h"
#include "Request.h"
#include "ring_buffer.h"
#include <arpa/inet.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
class ConnectionInfo {
public:
  int _fd;                   // 套接字描述符
  struct sockaddr_in _addr;  // 客户端地址信息
  ring_buffer _read_buffer;  // 读缓冲区
  ring_buffer _write_buffer; // 写缓冲区
  std::unique_ptr<REquest> _request;
  time_t _last_active_time; // 最后活跃时间戳
  //提供重置方法，用于重用连接信息
  void reset() {
    _fd = -1;
    _addr = {};
    _request.reset();
  }

  ~ConnectionInfo() {
    if (_fd >= 0) {
      close(_fd);
    }
    _request.reset();
  }
  int read() {
    int ret =
        recv(_fd, _read_buffer.get_buffer(), _read_buffer.not_useSize(), 0);
    if (ret < 0) {
      perror("read");
      return ret;
    }
    _read_buffer.writed(ret);          //更新读缓冲区
    _last_active_time = time(nullptr); //更新最后活跃时间戳
    return ret;
  }
  int write() {
    int ret =
        send(_fd, _write_buffer.get_buffer(), _write_buffer.used_size(), 0);
    _write_buffer.readed(ret);         //更新写缓冲区
    _last_active_time = time(nullptr); //更新最后活跃时间戳
    return ret;
  }
  //处理数据成请求
  bool convert_request() {
    //通过唯一工厂获取解析器，找到对应的解析器（解析成Request对象）并调用处理方法
    auto Facory = parserFactory::get_parser_factory();
    auto parser = Facory.get_parser(_read_buffer.get_buffer(),
                                    _read_buffer.not_useSize());
    if (parser == nullptr) {
      perror("no_parser");
      return false;
    }
    bool ret =
        parser->parse(_read_buffer.get_buffer(), _read_buffer.not_useSize());
    if (ret == false) {
      perror("parse_error");
      return false;
    }
    _request =
        parser->handle(_read_buffer.get_buffer(), _read_buffer.not_useSize());
    if (_request == nullptr) {
      perror("handle_error");
      return false;
    }
    return true;
  }
  //根据请求生成具体任务加入线程池队列
};

#endif