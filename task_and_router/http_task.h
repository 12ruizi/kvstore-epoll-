#pragma once
#include "../kv_core/kvstore.h"
#include "../network/connect_info/connect_info.h"
#include "Router.h"
#include <../network/epoll.h>
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

void set_write(ConnectionInfo *conn_info, const HttpResponse &response) {
  //这里需要做一个判断，判断用户态缓冲区大小是否足够，如果不够的
  //将响应状态行，响应头，响应体写入用户态缓冲区，那么这里可以用
}

//构建响应
HttpResponse build_response(const std::string &response_line,
                            const std::string &body) {
  HttpResponse response;
  response.response_line = response_line;
  response.body = body;
  return response;
}

std::string build_full_response(const HttpResponse &response) {
  return response.response_line + "\r\n" + "\r\n\r\n" +
         std::string(response.body.data(), response.body.size()) + "\r\n";
}

// 处理GET - url = /usr/zhang请求
void http_task_get1(ConnectionInfo *conn_info) {
  HttpResponse response;
  response.response_line = "HTTP/1.1 200 OK";
  response.body = "<h1>Hello, World!</h1>";
  send_simple_response(conn_info->_fd, response);
  std::cout << "response success" << std::endl;
};
void http_task_get2(ConnectionInfo *conn_info) {
  //得到conn_info->read_buffer.get_buffer()中的数据
  char *wmsg = conn_info->_write_buffer.get_buffer() +
               conn_info->_write_buffer.get_tail();
  std::cout << "wmsg = " << wmsg << std::endl;
  //这个需要只拿到数据部分，不包含协议部分
  char *rmsg_data = conn_info->_request->_data;
  std::cout << "rmsg_data = " << rmsg_data << std::endl;
  kvstore_request(rmsg_data, wmsg); //处理完协议

  HttpResponse response;
  response.response_line = "HTTP/1.1 200 OK";
  response.body = "<h1>set success</h1>";
  send_simple_response(conn_info->_fd, response);
}

//我已经得到http每一个部分的位置，只需要根据位置解析请求，构建响应

std::string http_build_response_line(int status_code,
                                     const std::string &reason_phrase) {
  return "HTTP/1.1 " + std::to_string(status_code) + " " + reason_phrase +
         "\r\n";
}
void http_build_headers(std::unordered_map<std::string, std::string> &headers,
                        std::string header, std::string headers_str) {
  headers[header] = headers_str;
}

void http_build_body(char *file, std::string msg) {}
REGISTER_HTTP_TASK("GET/usr/zhang", http_task_get1);
REGISTER_HTTP_TASK("POST/usr/kvstore", http_task_get2);
