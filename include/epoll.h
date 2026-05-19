
#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
//考虑设置哪些对外的api接口

class Epoll {
private:
  int _epoll_fd;
  int _max_events;
  std::vector<struct epoll_event> _events;

public:
  Epoll(int max_events) : _epoll_fd(epoll_create(1)), _max_events(max_events) {
    _events.resize(max_events);
    if (_epoll_fd < 0) {
      throw std::runtime_error("epoll_create failed");
    }
  };
  ~Epoll() {
    if (_epoll_fd >= 0) {
      close(_epoll_fd);
    }
    if (!_events.empty()) {
      _events.clear();
    }
  };
  Epoll(const Epoll &) = delete;
  Epoll &operator=(const Epoll &) = delete;
  //添加监听
  int add(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
  }
  //删除监听
  int del(int fd) { return epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL); }
  //修改监听事件
  int mod(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
  }
  //等待事件
  int wait(int timeout) {
    return epoll_wait(_epoll_fd, _events.data(), _max_events, timeout);
  }
  //获取事件列表
  std::vector<struct epoll_event> &events() { return _events; }
  //获取epollfd
  int fd() const {
    if (_epoll_fd < 0) {
      throw std::runtime_error("epoll fd is invalid");
    }
    return _epoll_fd;
  }
};
#endif