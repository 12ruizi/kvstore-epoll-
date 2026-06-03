#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include "kv_core/kvstore.h"
#include "log/Log.h"
#include "network/connect_info/Request.h"
#include "network/connect_info/connect_info.h"
#include "network/epoll.h"
#include "network/tcp.h"
#include "task_and_router/Router.h"
#include "timer/timer.h"
#include "utils/memory.h"
#include "utils/threads_pool.h"
#include <signal.h>
#include <stdexcept>
#define MAX_EVENTS 1024
#define PORT 2025
#define LOG_FLAG true
#define CLOCK_FLAG false
#define THREAD_POOL_FLAG false
#define TIME_OUT 1

class Webserver {
private:
  Epoll _epoll;          //需要max_events的参数,已经创建好epoll_fd了
  tcp_socket _tcp;       //需要port的参数,已经创建好tcp_fd了
  pthreads _thread_pool; //线程池
  Conn_pool _conn_Pool;  //对象连接池
  Thread_clockr _thread_clock; //定时器线程；
  static int *_pipe_fd;
  bool _stop_flag;

public:
  Webserver() : _epoll(MAX_EVENTS), _tcp(PORT), _stop_flag(false) {
    // 初始化管道
    _pipe_fd = new int[2];
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, _pipe_fd);
    if (ret < 0) {
      throw std::runtime_error("create socketpair failed");
    }

    // 将管道读端添加到epoll监听
    ret = _epoll.add(_pipe_fd[0], EPOLLIN);
    if (ret < 0) {
      throw std::runtime_error("add pipe fd to epoll failed");
    }

    // 添加SIGINT信号处理
    add_sig(SIGINT, sigint_handler, true);
    add_sig(SIGTERM, sigint_handler, true); // 同时处理SIGTERM

    ret = _epoll.add(_tcp.get_listen_fd(), EPOLLIN);
    if (ret < 0) {
      throw std::runtime_error("add epoll fd failed");
    }
    init_kvengine();
  }
  ~Webserver() {
    if (_pipe_fd) {
      close(_pipe_fd[0]);
      close(_pipe_fd[1]);
      std::cout << "close pipe fd" << std::endl;
      delete[] _pipe_fd;
      _pipe_fd = nullptr;
    }
    exit_kvengine();
  }

public:
  void EventLoop();
  int deal_NewConn();
  bool deal_Read(int conn_fd);
  bool deal_Write(int conn_fd);
  bool timer_task(int fd, time_t timeout);
  bool add_timer(int fd, time_t timeout, int version);
  bool del_all_timer();
  bool mod_timer(int fd, time_t timeout);
  bool del_timer(int fd);
  bool close_conn(int conn_fd);
  bool add_timer_log(int fd, time_t timeout, int version);
  bool del_signal(bool &stop_server);
  void add_sig(int signal, void(handler)(int), bool restart);
  static void sigint_handler(int signal) {
    int save_errno = errno;
    // 检查管道是否已初始化
    if (_pipe_fd != nullptr) {
      int message = signal;
      int sendfd = _pipe_fd[1];
      send(sendfd, (char *)&message, sizeof(message),
           0); // 发送整个整数值而不是单个字节
    }
    errno = save_errno;
  }
};

#endif