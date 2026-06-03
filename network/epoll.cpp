#include "epoll.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
//考虑设置哪些对外的api接口

Epoll::Epoll(int max_events)
    : _epoll_fd(epoll_create(1)), _max_events(max_events) {
  _events.resize(max_events);
  if (_epoll_fd < 0) {
    throw std::runtime_error("epoll_create failed");
  }
}

Epoll::~Epoll() {
  if (_epoll_fd >= 0) {
    close(_epoll_fd);
  }
  if (!_events.empty()) {
    _events.clear();
  }
}

//添加监听
int Epoll::add(int fd, uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  return epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

//删除监听
int Epoll::del(int fd) { return epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL); }

//修改监听事件
int Epoll::mod(int fd, uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  return epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

//等待事件
int Epoll::wait(int timeout) {
  return epoll_wait(_epoll_fd, _events.data(), _max_events, timeout);
}

//获取事件列表
std::vector<struct epoll_event> &Epoll::events() { return _events; }

//获取epollfd
int Epoll::get_epoll_fd() const {
  if (_epoll_fd < 0) {
    throw std::runtime_error("epoll fd is invalid");
  }
  return _epoll_fd;
}
