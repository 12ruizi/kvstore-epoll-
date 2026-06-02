#include "Webserver.h"
bool Webserver::deal_NewConn() {
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

  return true;
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
  if (ret < 0) {
    perror("read");
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
  }
  return true;
}

void Webserver::EventLoop() {
  std::cout << "server start" << std::endl;
  //可以加入日志记录
  LOG_INFO("server start");
  //事件循环
  while (1) {
    //等待事件
    int ret = epoll_wait(_epoll.get_epoll_fd(), _epoll.events().data(),
                         MAX_EVENTS, -1);
    if (ret < 0) {
      perror("epoll_wait");
      continue;
    }
    for (auto &Event : _epoll.events()) {
      if (Event.data.fd == _tcp.get_listen_fd()) {
        deal_NewConn();
        add_timer(Event.data.fd, TIME_OUT, 1);
        LOG_INFO("add_timer conn fd: %d", Event.data.fd);
      } else if (Event.events & EPOLLIN) {
        deal_Read(Event.data.fd);
        mod_timer(Event.data.fd, TIME_OUT);
        LOG_INFO("mod_timer conn fd: %d", Event.data.fd);
      } else if (Event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        //关闭连接和移除定时器
        close_conn(Event.data.fd);
        if (LOG_FLAG) {
          LOG_ERROR("close conn fd: %d", Event.data.fd);
        }
      }
    }
  }
};

bool Webserver::timer_task(int fd, time_t timeout) {
  //在timer实例中添加定时器，超时时间为timeout
  // fd最后的活跃时间和当前时间的差值大于timeout，说明连接超时
  auto conn = _conn_Pool.get(fd);
  if (conn == nullptr) {
    perror("get conn from memory_pool failed");
    return false;
  }
  if (conn->_last_active_time + timeout < time(nullptr)) {
    //连接超时
    LOG_ERROR("conn fd: %d timeout", fd);
    std::cout << "conn fd: " << fd << " timeout" << std::endl;
    close_conn(fd);
    return true;
  }
  return false;
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
  close(conn->_fd);
  _conn_Pool.Release(conn);
  _epoll.del(conn_fd);
  del_timer(conn_fd);
  LOG_INFO("close conn fd: %d", conn_fd);
  return true;
}
bool Webserver::add_timer_log(int fd, time_t timeout, int version) {
  return _thread_clock.add_timer(
      fd, timeout,
      [fd]() {
        std::cout << "now:" << time(nullptr) << ":" << fd << std::endl;
      },
      version);
}
