#ifndef __PROTOCOL_PARSER_H__
#define __PROTOCOL_PARSER_H__
#include "Request.h"
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
  bool parse(ConnectionInfo *info) override;

  bool handle(ConnectionInfo *info) override;
};

class parserFactory { //协议解析器工厂类
private:
  std::map<std::string, std::function<std::unique_ptr<ProtocolParser>()>>
      _registry_map_;
  size_t _max_signature_length = 20;
  parserFactory() = default;

public:
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

#define REGISTER_PARSER(name, creator)                                         \
  static std::unique_ptr<ProtocolParser> creator##Factory() {                  \
    return std::make_unique<creator>();                                        \
  }                                                                            \
                                                                               \
  static struct Register_##name {                                              \
    Register_##name() {                                                        \
      parserFactory::get_parser_factory().register_parser(#name,               \
                                                          creator##Factory);   \
      std::cout << "自动注册协议解析器: " << #name << std::endl;               \
    }                                                                          \
  } g_register_##name;

#endif // __PROTOCOL_PARSER_H__