#ifndef __SERVER_H__
#define __SERVER_H__

#include "../kv_core/kvstore.h"
#include "../log/log.h"
#include "../network/connect_info/Request.h"
#include "../network/connect_info/connect_info.h"
#include "../network/epoll.h"
#include "../network/protocol_parser/protocol_parser.h"
#include "../network/tcp.h"
#include "../task_and_router/Router.h"
#include "../timer/timer.h"
#include "../utils/threads_pool.h"
#include <sys/epoll.h>

#define MAX_EVENTS 1024
#define DEFAULT_PORT 2025

class Server {
private:
  Epoll _epoll;
  tcp_socket _tcp;
  pthreads _threads;           //工作线程
  Thread_clockr _timer_thread; //定时器线程
  // ProtocolFactory _factory;

public:
  Server();
  ~Server();
  int run();
};

#endif // __SERVER_H__