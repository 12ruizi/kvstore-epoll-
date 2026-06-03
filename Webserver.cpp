#include "Webserver.h"

int Webserver::deal_NewConn() {
  int conn_fd = _tcp.Accept_tcp();
  if (conn_fd < 0) {
    perror("accept");
    return false;
  }
  _epoll.add(conn_fd, EPOLLIN);
  //申请连接对象,初始化连接信息
  auto conn = _conn_Pool.acquire();
  if (conn == nullptr) {
    perror("acquire conn from memory_pool failed");
    return false;
  }
  _conn_Pool.add_map(conn_fd, conn);

  return conn_fd;
};
bool Webserver::deal_Read(int conn_fd) {

  //从连接池获取连接对象
  auto conn = _conn_Pool.get(conn_fd);
  if (conn == nullptr) {
    perror("get conn from memory_pool failed");
    return false;
  }

  LOG_INFO("deal_Read conn fd: %d", conn_fd);
  //加入定时器

  //读取数据到该连接的缓冲区
  int ret = conn->read();
  if (ret <= 0) {
    close_conn(conn_fd);
    LOG_ERROR("close conn fd: %d", conn_fd);
    return false;
  }
  //处理数据成request
  bool success = conn->convert_request();
  if (success == false) {
    perror("convert_request failed");
    return false;
  }
  //根据请求生成具体任务加入线程池队列
  auto handler = Router::instance().get_handler(conn->_request->get_key_URL());
  if (handler == nullptr) {
    perror("get handler failed");
    return false;
  }
  //加入线程池队列

  if (THREAD_POOL_FLAG) {
    _thread_pool.enqueue(handler, conn);
  } else {
    handler(conn);
    conn->_last_active_time = time(nullptr);
  }
  return true;
}

void Webserver::add_sig(int signal, void(handler)(int), bool restart) {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = handler;
  if (restart) {
    sa.sa_flags |= SA_RESTART;
  }
  sigfillset(&sa.sa_mask);
  assert(sigaction(signal, &sa, NULL) != -1);
}
int *Webserver::_pipe_fd = nullptr;
void Webserver::EventLoop() {
  std::cout << "server start" << std::endl;
  //可以加入日志记录
  LOG_INFO("server start");
  //事件循环
  while (!_stop_flag) {
    //等待事件，设置超时时间以便检查stop_flag
    int ret = epoll_wait(_epoll.get_epoll_fd(), _epoll.events().data(),
                         MAX_EVENTS, 100); // 1秒超时
    if (ret < 0) {
      if (errno == EINTR)
        continue; // 被信号中断，继续循环
      perror("epoll_wait");
      continue;
    }
    if (ret == 0)
      continue; // 超时，继续循环检查stop_flag
    for (int i = 0; i < ret; i++) {
      auto &Event = _epoll.events()[i];
      if (_stop_flag)
        break; // 再次检查，以防在处理事件时收到信号
      if (Event.data.fd == _tcp.get_listen_fd()) {
        int conn_fd = deal_NewConn();
        //将新连接的fd加入定时器，超时时间为TIME_OUT
        add_timer(conn_fd, TIME_OUT, 1);
        LOG_INFO("add_timer conn fd: %d", conn_fd);
      } else if (Event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        //关闭连接和移除定时器
        close_conn(Event.data.fd);
        if (LOG_FLAG) {
          LOG_ERROR("close conn fd: %d", Event.data.fd);
        }
      } else if (Event.data.fd == _pipe_fd[0] && Event.events & EPOLLIN) {
        bool success = del_signal(_stop_flag);
        if (!success) {
          LOG_ERROR("del_signal failed");
        }
      }

      else if (Event.events & EPOLLIN) {
        deal_Read(Event.data.fd);
        mod_timer(Event.data.fd, TIME_OUT);
        LOG_INFO("mod_timer conn fd: %d", Event.data.fd);
      }
    }
  }
  std::cout << "\nReceived signal, shutting down gracefully..." << std::endl;
};

bool Webserver::add_timer(int fd, time_t timeout, int version) {
  int ret = _thread_clock.add_timer(
      fd, timeout, std::bind(&Webserver::timer_task, this, fd, timeout),
      version);
  return ret > 0;
};
bool Webserver::del_all_timer() { return _thread_clock.del_all_timer(); };
bool Webserver::mod_timer(int fd, time_t timeout) {
  return _thread_clock.mod_timer(
      fd, timeout, [this, fd, timeout]() { timer_task(fd, timeout); });
}
bool Webserver::del_timer(int fd) { return _thread_clock.del_timer(fd); }
bool Webserver::close_conn(int conn_fd) {
  auto conn = _conn_Pool.get(conn_fd);
  if (conn == nullptr) {
    perror("get conn from memory_pool failed");
    return false;
  }

  _epoll.del(conn_fd);
  del_timer(conn_fd);
  _conn_Pool.Release(conn);
  LOG_INFO("close conn fd: %d", conn_fd);
  return true;
}
bool Webserver::timer_task(int fd, time_t timeout) {
  //在timer实例中添加定时器，超时时间为timeout
  // fd最后的活跃时间和当前时间的差值大于timeout，说明连接超时
  auto conn = _conn_Pool.get(fd);
  if (conn == nullptr) {
    perror("get conn from memory_pool failed");
    return false;
  }
  if (conn->_last_active_time + timeout <= time(nullptr)) {
    //连接超时
    LOG_ERROR("conn fd: %d timeout", fd);
    close_conn(fd);
    return true;
  }
  return false;
};
bool Webserver::add_timer_log(int fd, time_t timeout, int version) {
  return _thread_clock.add_timer(
      fd, timeout,
      [fd]() {
        std::cout << "now:" << time(nullptr) << ":" << fd << std::endl;
      },
      version);
}
bool Webserver::del_signal(bool &stop_server) {
  int ret = 0;
  int signals[1024]; // 使用int数组而不是char数组，因为发送的是整数信号值

  ret = recv(_pipe_fd[0], signals, sizeof(signals), 0);
  if (ret == -1) {
    return false;
  } else if (ret == 0) {
    return false;
  } else {
    int num_ints = ret / sizeof(int);
    for (int i = 0; i < num_ints; ++i) {
      switch (signals[i]) {
      case SIGTERM:
      case SIGINT: {
        stop_server = true;
        break;
      }
      }
    }
  }

  return true;
}