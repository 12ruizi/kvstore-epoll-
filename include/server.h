#ifndef __SERVER_H__
#define __SERVER_H__

#include "connect_info.h"
#include "epoll.h"
#include "kvstore.h"
#include "protocol_parser.h"
#include "tcp.h"
#include <sys/epoll.h>
#define MAX_EVENTS 1024
#define DEFAULT_PORT 2025
class Server {
private:
  Epoll _epoll;
  tcp_socket _tcp;
  // ProtocolFactory _factory;
public:
  Server() : _epoll(MAX_EVENTS), _tcp(DEFAULT_PORT){};
  ~Server(){};
  //初始化tcp
  //初始化epoll
  //事件循环
  int run() {

    if (_tcp.fd() < 0) {
      throw std::runtime_error("tcp socket create failed");
    }
    if (_epoll.fd() < 0) {
      throw std::runtime_error("epoll create failed");
    }
    //添加tcp socket到epoll
    int set = _epoll.add(_tcp.fd(), EPOLLIN);
    if (set < 0) {
      throw std::runtime_error("epoll add tcp socket failed");
    }
    auto &factory = parserFactory::get_parser_factory();
    //事件循环
    while (1) {
      int ret = epoll_wait(_epoll.fd(), _epoll.events().data(), MAX_EVENTS, -1);
      if (ret < 0) {
        perror("epoll_wait");
        return -1;
      }
      for (auto &Event : _epoll.events()) {
        if (Event.data.fd == _tcp.fd()) {
          int conn_fd = _tcp.Accept_tcp();
          if (conn_fd < 0) {
            perror("Accept_tcp");
            continue;
          }
          int ret = _epoll.add(conn_fd, EPOLLIN);
          if (ret < 0) {
            perror("epoll add conn socket failed");
            close(conn_fd);
            continue;
          }

        } else {
          //处理连接的事件
          switch (Event.events) {
          case EPOLLIN: {
            //读取数据到连接池对象的rbuffer
            auto conn_info = _tcp._conn_pool.get(Event.data.fd);
            if (conn_info == nullptr) {
              perror("get conn info failed");
              continue;
            }
            int ret = recv(conn_info->fd, conn_info->read_buffer.get_buffer(),
                           conn_info->read_buffer.not_useSize(), 0);
            if (ret < 0) {
              perror("recv");
              continue;
            }
            conn_info->read_buffer.writed(ret);
            auto parse = factory.get_parser(*conn_info);
            //读取数据
            if (parse == nullptr) {
              continue;
            }
            if (!parse->parse(conn_info)) {
              continue;
            }
            parse->handle(conn_info); //这里是直接处理了，但是可以让线程池区处理
            //协议解析的处理
            break;
          }

          case EPOLLOUT: {
            //写回数据
            break;
          }
          default: {
            //其他事件
            break;
          }
          }
        }
      }
    };
  };
};
#endif