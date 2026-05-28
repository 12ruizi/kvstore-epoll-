#pragma once
#include "Router.h"
#include "connect_info.h"
#include <string>
#include <sys/socket.h>
// HTTP响应结构体
struct HttpResponse {
  std::string response_line;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
  HttpResponse() {
    headers["Content-Type"] = "text/html; charset=utf-8";
    headers["Connection"] = "keep-alive";
  }
};

// 发送简单响应
void send_simple_response(size_t fd, const HttpResponse &response) {
  std::string headers_str;
  for (const auto &header : response.headers) {
    headers_str += header.first + ": " + header.second + "\r\n";
  }
  std::string full_response =
      response.response_line + headers_str + "\r\n" + "\r\n\r\n" +
      std::string(response.body.data(), response.body.size()) + "\r\n";
  int ret = send(fd, full_response.c_str(), full_response.size(), 0);
  std::cout << "send full_response = " << full_response << std::endl;

  if (ret < 0) {
    perror("send");
  }
};
// 处理GET - url = /usr/zhang请求
void http_task_get1(ConnectionInfo *conn_info) {
  HttpResponse response;
  response.response_line = "HTTP/1.1 200 OK";
  response.body = "<h1>Hello, World!</h1>";
  send_simple_response(conn_info->fd, response);
  std::cout << "http_task_get1_success" << std::endl;
};
REGISTER_HTTP_TASK("GET/usr/zhang", http_task_get1);
