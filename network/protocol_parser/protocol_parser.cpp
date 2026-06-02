#include "protocol_parser.h"
#include "../../task_and_router/http_task.h"
#include "../connect_info/connect_info.h"
#include <algorithm>
#include <iostream>

// 定义静态成员函数
parserFactory &parserFactory::get_parser_factory() {
  static parserFactory parser_factory;
  return parser_factory;
}

// 注册解析器
REGISTER_PARSER(POST, HttpParser);
REGISTER_PARSER(GET, HttpParser);

ParseResult HttpParser::is_complete_message(char *buffer, size_t buffer_size) {
  if (buffer_size == 0) {
    return ParseResult::NEEED_MORE_DATA;
  }
  std::string_view request(buffer, buffer_size);

  // 查找请求头结束标记
  size_t headers_end = request.find("\r\n\r\n");
  if (headers_end == std::string_view::npos) {
    return ParseResult::NEEED_MORE_DATA;
  }

  // 解析Content-Length
  size_t content_length_pos = request.find("Content-Length:");
  if (content_length_pos != std::string_view::npos &&
      content_length_pos < headers_end) {
    size_t content_length_end = request.find("\r\n", content_length_pos);
    if (content_length_end != std::string_view::npos) {
      std::string content_len_str = std::string(
          request.substr(content_length_pos + 15,
                         content_length_end - (content_length_pos + 15)));
      // 去除空格
      content_len_str.erase(0, content_len_str.find_first_not_of(" \t"));
      content_len_str.erase(content_len_str.find_last_not_of(" \t") + 1);

      size_t content_len = std::stoul(content_len_str);
      if (buffer_size >= headers_end + 4 + content_len) {
        return ParseResult::COMPLETE;
      } else {
        return ParseResult::NEEED_MORE_DATA;
      }
    }
  }
  return ParseResult::COMPLETE;
}

//头部指针，数据指针，
char *HttpParser::Head(char *buffer, size_t bufferSize) {
  std::string_view request(buffer, bufferSize);
  size_t headers_end = request.find("\r\n\r\n");
  if (headers_end != std::string_view::npos) {
    return buffer;
  }
  return nullptr;
}

//数据指针
char *HttpParser::Data(char *buffer, size_t bufferSize) {
  std::string_view request(buffer, bufferSize);
  size_t headers_end = request.find("\r\n\r\n");
  if (headers_end != std::string_view::npos) {
    return buffer + headers_end + 4;
  }
  return nullptr;
}

//请求方法
std::string HttpParser::Method(char *buffer, size_t bufferSize) {
  std::string_view request(buffer, bufferSize);

  size_t method_end = request.find(' ');
  if (method_end != std::string_view::npos) {
    return std::string(request.substr(0, method_end));
  }
  return "";
}

//请求url
std::string HttpParser::URL(char *buffer, size_t bufferSize) {
  std::string_view request(buffer, bufferSize);

  size_t method_end = request.find(' ');
  if (method_end != std::string_view::npos) {
    size_t url_end = request.find(' ', method_end + 1);
    if (url_end != std::string_view::npos) {
      return std::string(
          request.substr(method_end + 1, url_end - method_end - 1));
    }
  }
  return "";
}

bool HttpParser::parse(char *buffer, size_t bufferSize) {
  ParseResult result = is_complete_message(buffer, bufferSize);
  if (result == ParseResult::NEEED_MORE_DATA) {
    return false;
  }
  return true;
}

std::unique_ptr<REquest> HttpParser::handle(char *buffer, size_t bufferSize) {
  std::cout << "http parser handle" << std::endl;
  auto request = std::make_unique<HttpRequest>();
  request->_method = Method(buffer, bufferSize);
  request->_url = URL(buffer, bufferSize);
  request->_version = "HTTP/1.1";
  request->_data = Data(buffer, bufferSize);
  std::cout << "http" << std::endl;
  return request;
}

//注册协议解析器
void parserFactory::register_parser(
    const std::string &protocol_name,
    std::function<std::unique_ptr<ProtocolParser>()> creator) {
  _registry_map_[protocol_name] = creator;
  std::cout << "已经注册协议特征编码：" << protocol_name << std::endl;
}

std::unique_ptr<ProtocolParser> parserFactory::get_parser(char *buffer,
                                                          size_t bufferSize) {
  if (bufferSize == 0) {
    std::cout << "read_buffer is empty" << std::endl;
    return nullptr;
  }
  size_t len = std::min(static_cast<size_t>(bufferSize), _max_signature_length);
  for (; len > 0; --len) {
    //字符串视图
    std::string_view prefix(buffer, len);
    std::cout << "prefix :" << prefix << std::endl;
    //这里缀，可能特征缀，可能特征在前缀的后面
    auto it = _registry_map_.find(std::string(prefix));
    if (it != _registry_map_.end()) {
      return it->second();
    }
  }
  std::cout << "no parser found" << std::endl;
  return nullptr;
}