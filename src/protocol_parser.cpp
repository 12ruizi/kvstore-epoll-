#include "protocol_parser.h"
#include "connect_info.h"
#include "http_task.h"
#include <algorithm>
#include <iostream>
REGISTER_PARSER(GET, HttpParser);
ParseResult HttpParser::is_complete_message(ConnectionInfo *info) {
  size_t read_size = info->read_buffer.used_size();
  if (read_size == 0) {
    return ParseResult::NEEED_MORE_DATA;
  }
  std::string_view request(
      info->read_buffer.get_buffer() + info->read_buffer.get_head(), read_size);

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
      if (read_size >= headers_end + 4 + content_len) {
        return ParseResult::COMPLETE;
      } else {
        return ParseResult::NEEED_MORE_DATA;
      }
    }
  }
  return ParseResult::COMPLETE;
}
//头部指针，数据指针，
char *HttpParser::Head(ConnectionInfo *info) {
  std::string_view request(info->read_buffer.get_buffer() +
                               info->read_buffer.get_head(),
                           info->read_buffer.used_size());
  size_t headers_end = request.find("\r\n\r\n");
  if (headers_end != std::string_view::npos) {
    return info->read_buffer.get_buffer() + info->read_buffer.get_head();
  }
  return nullptr;
}
//数据指针
char *HttpParser::Data(ConnectionInfo *info) {
  std::string_view request(info->read_buffer.get_buffer() +
                               info->read_buffer.get_head(),
                           info->read_buffer.used_size());
  size_t headers_end = request.find("\r\n\r\n");
  if (headers_end != std::string_view::npos) {
    return info->read_buffer.get_buffer() + info->read_buffer.get_head() +
           headers_end + 4;
  }
  return nullptr;
}
//请求方法
std::string HttpParser::Method(ConnectionInfo *info) {
  std::string_view request(info->read_buffer.get_buffer() +
                               info->read_buffer.get_head(),
                           info->read_buffer.used_size());
  size_t method_end = request.find(' ');
  if (method_end != std::string_view::npos) {
    return std::string(request.substr(0, method_end));
  }
  return "";
}
//请求url
std::string HttpParser::URL(ConnectionInfo *info) {
  std::string_view request(info->read_buffer.get_buffer() +
                               info->read_buffer.get_head(),
                           info->read_buffer.used_size());
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

bool HttpParser::parse(ConnectionInfo *info) {
  ParseResult result = is_complete_message(info);
  if (result == ParseResult::NEEED_MORE_DATA) {

    return false;
  }
  return true;
}

bool HttpParser::handle(ConnectionInfo *info) {
  std::cout << "http parser handle" << std::endl;
  auto requeset = std::make_unique<HttpRequest>();
  requeset->_method = Method(info);
  requeset->_url = URL(info);
  requeset->_version = "HTTP/1.1";
  requeset->_data = Data(info);
  info->_request = std::move(requeset);
  std::cout << "http" << std::endl;
  return true;
};

//注册协议解析器
void parserFactory::register_parser(
    const std::string &protocol_name,
    std::function<std::unique_ptr<ProtocolParser>()> creator) {
  _registry_map_[protocol_name] = creator;
  std::cout << "已经注册协议特征编码：" << protocol_name << std::endl;
}

std::unique_ptr<ProtocolParser>
parserFactory::get_parser(ConnectionInfo &info) {
  if (info.read_buffer.used_size() == 0) {
    std::cout << "read_buffer is empty" << std::endl;
    return nullptr;
  }
  size_t len = std::min(static_cast<size_t>(info.read_buffer.used_size()),
                        _max_signature_length);
  for (; len > 0; --len) {
    //字符串视图
    std::string_view prefix(
        info.read_buffer.get_buffer() + info.read_buffer.get_head(), len);
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
