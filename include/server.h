#ifndef __SERVER_H__
#define __SERVER_H__
#include "../kv_core/kvstore.h"
#include "../log/log.h"
#include "Request.h"
#include "Router.h"
#include "connect_info.h"
#include "epoll.h"
#include "kvstore.h"
#include "protocol_parser.h"
#include "tcp.h"
#include "threads_pool.h"
#include <sys/epoll.h>
#define MAX_EVENTS 1024
#define DEFAULT_PORT 2025
class Server {
private:
  Epoll _epoll;
  tcp_socket _tcp;
  pthreads _threads;

  // ProtocolFactory _factory;
public:
  Server() : _epoll(MAX_EVENTS), _tcp(DEFAULT_PORT), _threads() {
    kv_log::get_instance()->init("kvstore.log");
    init_kvengine();
  };
  ~Server() {
    if (_epoll.fd() >= 0) {
      close(_epoll.fd());
    }
    if (_tcp.fd() >= 0) {
      close(_tcp.fd());
    }
    exit_kvengine();
  };
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
    std::cout << "server start" << std::endl;
    kv_log::get_instance()->write_log(0, "server start");
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
            std::cout << "close conn_fd = " << conn_fd << std::endl;
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
            std::cout << "recv :" << conn_info->read_buffer.get_buffer()
                      << std::endl;
            if (ret < 0) {
              perror("recv");
              continue;
            }
            conn_info->read_buffer.writed(ret);
            auto parse = factory.get_parser(*conn_info);
            //读取数据
            if (parse == nullptr) {
              perror("get parser failed");
              continue;
            }
            if (!parse->parse(conn_info)) {
              perror("parse failed");
              continue; //数据不完整
            }
            std::cout << "parse success111" << std::endl;
            std::cout << "开始处理handle" << std::endl;
            parse->handle(conn_info);
            //-----这里选择忽略任务返回值，
            _threads.enqueue([conn_info]() -> std::string {
              auto &request = conn_info->_request;
              auto key_URL = request->get_key_URL();
              std::cout << "key_URL :" << key_URL << std::endl;
              auto handler = Router::instance().get_handler(key_URL);
              if (handler == nullptr) {
                perror("get handler failed");
                return "404 Not Found";
              }
              handler(conn_info);
              return "200 OK";
            }); //处理成请求

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