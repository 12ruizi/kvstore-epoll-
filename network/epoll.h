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
  Epoll(int max_events);
  ~Epoll();
  Epoll(const Epoll &) = delete;
  Epoll &operator=(const Epoll &) = delete;
  //添加监听
  int add(int fd, uint32_t events);
  //删除监听
  int del(int fd);
  //修改监听事件
  int mod(int fd, uint32_t events);
  //等待事件
  int wait(int timeout);
  //获取事件列表
  std::vector<struct epoll_event> &events();
  //获取epollfd
  int fd() const;
};
#endif