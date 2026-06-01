#ifndef ROUTER_H
#define ROUTER_H
#include "../network/connect_info/connect_info.h"
#include <functional>
#include <map>
#include <string>
typedef struct router {
  std::string _key;
  bool operator<(const router &other) const {
    if (_key != other._key) {

      return _key < other._key;
    }

    return false;
  }

} router_t;
class Router {
private:
  std::map<router_t, std::function<void(ConnectionInfo *)>> _router_map;

public:
  static Router &instance() {
    static Router R;
    return R;
  }

  void register_router(std::string key_URL,
                       std::function<void(ConnectionInfo *)> conn) {
    _router_map[router_t{key_URL}] = conn;
  }
  //返回对应处理的函数
  std::function<void(ConnectionInfo *)>
  get_handler(const std::string &key_URL) {
    auto it = _router_map.find(router_t{key_URL});
    if (it == _router_map.end()) {
      return nullptr;
    }
    return it->second;
  }
};
#define REGISTER_HTTP_TASK(key_URL, handler)                                   \
  static struct Registrar_##handler {                                          \
    Registrar_##handler() {                                                    \
      Router::instance().register_router(key_URL, handler);                    \
      std::cout << "register router " << key_URL << std::endl;                 \
    }                                                                          \
  } g_registrar_##handler;
#endif