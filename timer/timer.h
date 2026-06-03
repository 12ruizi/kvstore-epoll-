#ifndef TIMER_H
#define TIMER_H
//用最小堆或者红黑树实现定时器
// stl中的priority_queue可以实现最小堆
// stl中的set map底层是红黑树
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unordered_map>
#include <vector>
//定时器结构体
struct timer {
  int timer_id;                   //定时器id
  time_t expire_time;             //过期的时间戳
  std::function<void()> callback; //定时器回调函数
  int version;                    //是否是最新的定时器
  bool operator>(const timer &other) const;
  timer(int timer_id, time_t expire_time = 0,
        std::function<void()> callback = nullptr, int version = 0);
};

class Thread_clockr {
private:
  std::priority_queue<std::unique_ptr<timer>,
                      std::vector<std::unique_ptr<timer>>,
                      decltype([](const std::unique_ptr<timer> &a,
                                  const std::unique_ptr<timer> &b) {
                        return a->expire_time > b->expire_time;
                      })>
      _timer_queue; //定时器队列

  //保存每个id最新的容器
  std::unordered_map<int, int> _timer_map;
  std::condition_variable _cond_timer;
  std::thread _timer_thread;
  std::mutex _mutex; //互斥锁
  bool _stop_flag;   // 添加停止标志

  //处理快要到时间点的定时器
  void handle_timer();

public:
  //添加定时器//跟连接id一样，
  int add_timer(int timer_id, time_t without, std::function<void()> callback,
                int version);
  //删除所有定时器
  bool del_all_timer();
  //修改定时器
  bool mod_timer(int timer_id, time_t without, std::function<void()> callback);
  //删除定时器
  bool del_timer(int timer_id);
  Thread_clockr();
  ~Thread_clockr(

  );
};
//底层是最小堆，那么让一个线程去做 每次添加定时器加锁
#endif