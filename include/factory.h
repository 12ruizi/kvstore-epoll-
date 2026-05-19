#pragma once
//有一个连接池，每次来新的连接就把 连接放到连接池中
#include "connect_info.h"
#include <memory>
#include <string>
//基类任务，从socket中读取数据到用户态缓冲区
class task {
public:
  //需要解析报文内容
  virtual int cpy_read_buffer(ConnectionInfo *conn) {
    int ret = recv(conn->fd, conn->read_buffer.get_buffer(),
                   conn->read_buffer.not_useSize(), 0);
    if (ret < 0) {
      perror("recv");
      return -1;
    }
    conn->read_buffer.writed(ret);
    return ret;
  }
  virtual int cpy_write_buffer(ConnectionInfo *conn) {
    int ret = send(conn->fd, conn->write_buffer.get_buffer(),
                   conn->write_buffer.used_size(), 0);
    if (ret < 0) {
      perror("send");
      return -1;
    }
    conn->write_buffer.readed(ret);
    return ret;
  }

  virtual ~task();
};
class HttpTask : public task {
public:
  ~HttpTask(){};
  //解析报文内容处理
  int parse_http(ConnectionInfo *conn);
  //
};
class kv_task : public task {
public:
  ~kv_task(){};
  //解析报文内容处理
  int parse_kv(ConnectionInfo *conn);
  //
};
class TaskFactory {
public:
  virtual ~TaskFactory(){};
  virtual std::unique_ptr<task> createTask() = 0;
};
class HTTpTaskFactory : public TaskFactory {
public:
  ~HTTpTaskFactory(){};
  std::unique_ptr<task> createTask() override {
    return std::make_unique<HttpTask>();
  }
};
class kv_taskFactory : public TaskFactory {
public:
  ~kv_taskFactory(){};
  std::unique_ptr<task> createTask() override {
    return std::make_unique<kv_task>();
  }
};
