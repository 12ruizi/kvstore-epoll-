#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__
#include "log/Log.h"
#include "network/connect_info/Request.h"
#include "network/connect_info/connect_info.h"
#include "network/epoll.h"
#include "network/tcp.h"
#include "task_and_router/Router.h"
#include "timer/timer.h"
#include "utils/memory.h"
#include "utils/threads_pool.h"
#include <stdexcept>
#define MAX_EVENTS 1024
#define PORT 2025
#define LOG_FLAG true
#define CLOCK_FLAG false
#define THREAD_POOL_FLAG false
#define TIME_OUT 10

class Webserver {
private:
  Epoll _epoll;          //需要max_events的参数,已经创建好epoll_fd了
  tcp_socket _tcp;       //需要port的参数,已经创建好tcp_fd了
  pthreads _thread_pool; //线程池
  Conn_pool _conn_Pool;  //对象连接池
  Thread_clockr _thread_clock; //定时器线程；
  bool stop_flag = false;

public:
  Webserver() : _epoll(MAX_EVENTS), _tcp(PORT) {
    int ret = _epoll.add(_tcp.get_listen_fd(), EPOLLIN);
    if (ret < 0) {
      throw std::runtime_error("add epoll fd failed");
    }
  }
  void EventLoop();
  bool deal_NewConn();
  bool deal_Read(int conn_fd);
  bool deal_Write(int conn_fd);
  bool timer_task(int fd, time_t timeout);
  bool add_timer(int fd, time_t timeout, int version);
  bool del_all_timer();
  bool mod_timer(int fd, time_t timeout);
  bool del_timer(int fd);
  bool close_conn(int conn_fd);
  bool add_timer_log(int fd, time_t timeout, int version);
};

#endif