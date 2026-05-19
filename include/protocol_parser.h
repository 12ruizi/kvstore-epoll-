#ifndef __PROTOCOL_PARSER_H__
#define __PROTOCOL_PARSER_H__
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
class ConnectionInfo;
enum ParseResult {
  NEEED_MORE_DATA,
  COMPLETE,
};
struct http_method_Url {
  std::string method;
  std::string url;
  bool operator<(const http_method_Url &other) const {
    if (method != other.method) {
      return method < other.method;
    } else {
      return url < other.url;
    }
  };
};
//数据流的处理（协议的解析）
//数据流的调度器 利用策略模式来处理不同的任务
// #define REGISTER_PARSER(signature, parser_class) \
//   static bool register_##parser_class = []() { \
//     parserFactory::get_parser_factory().register_parser( \
//         signature, []() { return std::make_shared<parser_class>(); }); \
//     return true; \
//   }();

class ProtocolParser { //协议解析基类
public:
  virtual ~ProtocolParser(){};

  virtual bool parse(ConnectionInfo *info) = 0;
  virtual bool handle(ConnectionInfo *info) = 0;
};

class HttpParser : public ProtocolParser {
private:
  ParseResult is_complete_message(ConnectionInfo *info);
  //头部指针，数据指针，
  char *Head(ConnectionInfo *info);
  //数据指针
  char *Data(ConnectionInfo *info);
  //请求方法
  std::string Method(ConnectionInfo *info);

  //请求url
  std::string URL(ConnectionInfo *info);

public:
  static std::map<http_method_Url, std::function<void()>>
      HTTP_handler_map; // HTTP请求处理函数映射

  static bool register_handler(const std::string &method,
                               const std::string &url,
                               std::function<void()> handler) {
    if (method.empty() || url.empty()) {
      return false;
    }
    std::unique_ptr<http_method_Url> key = std::make_unique<http_method_Url>();
    key->method = method;
    key->url = url;
    HTTP_handler_map[*key] = handler;
    std::cout << "已经注册HTTP方法的处理函数：" << method << std::endl;
    return true;
  }
  bool parse(ConnectionInfo *info) override;
  bool handle(ConnectionInfo *info) override;
};

class parserFactory { //协议解析器工厂类
private:
  std::map<std::string, std::function<std::unique_ptr<ProtocolParser>()>>
      _registry_map_;
  size_t _max_signature_length;
  parserFactory() = default;

public:
  static std::map<http_method_Url, std::function<void()>>
      HTTP_handler_map; // HTTP请求处理函数映射
  //获取全局唯一的工厂实例
  static parserFactory &get_parser_factory() {
    static parserFactory parser_factory;
    return parser_factory;
  }
  //注册协议解析器
  void
  register_parser(const std::string &protocol_name,
                  std::function<std::unique_ptr<ProtocolParser>()> creator);
  std::unique_ptr<ProtocolParser> get_parser(ConnectionInfo &info);
};
#endif // __PROTOCOL_PARSER_H__
