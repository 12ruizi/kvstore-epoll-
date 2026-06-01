#include "timer.h"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unordered_map>
#include <vector>

//定时器结构体
timer::timer(int timer_id, time_t expire_time, std::function<void()> callback,
             int version)
    : timer_id(timer_id), expire_time(expire_time), callback(callback),
      version(version) {}

bool timer::operator>(const timer &other) const {
  return expire_time > other.expire_time;
}

//添加定时器//跟连接id一样，
void Thread_clockr::add_timer(int timer_id, time_t without,
                              std::function<void()> callback, int version) {
  std::lock_guard<std::mutex> lock(_mutex);
  time_t expire_time = time(nullptr) + without;
  auto T = std::make_unique<timer>(timer_id, expire_time, callback, version);
  _timer_queue.push(std::move(T));
  _cond_timer.notify_one();
}

//删除所有定时器
void Thread_clockr::del_all_timer() {
  std::lock_guard<std::mutex> lock(_mutex);
  while (!_timer_queue.empty()) {
    _timer_queue.pop();
  }
}

//修改定时器
void Thread_clockr::mod_timer(int timer_id, time_t without,
                              std::function<void()> callback) {
  std::lock_guard<std::mutex> lock(_mutex);
  _timer_map[timer_id]++;
  int new_version = _timer_map[timer_id];
  //添加新的定时器
  add_timer(timer_id, without, callback, new_version);
}

//处理快要到时间点的定时器
void Thread_clockr::handle_timer() {
  std::lock_guard<std::mutex> lock(_mutex);
  time_t now = time(nullptr);
  //判断是否有新的定时器是否过期
  while (!_timer_queue.empty()) {
    if (_timer_queue.top()->expire_time > now) {
      break;
    }
    if (_timer_map.count(_timer_queue.top()->timer_id) &&
        _timer_map[_timer_queue.top()->timer_id] !=
            _timer_queue.top()->version) {
      _timer_queue.pop(); // 跳过过时的定时器
      continue;
    }
    if (_timer_queue.top()->callback) {
      _timer_queue.top()->callback();
    }
    _timer_queue.pop();
  }
}

Thread_clockr::Thread_clockr() {
  _timer_thread = std::thread([this]() {
    while (true) {
      std::unique_lock<std::mutex> lock(_mutex);
      //查看堆顶的时间戳和现在的时间戳计算等待时间

      int wait_time = 0;
      if (!_timer_queue.empty()) {
        wait_time = _timer_queue.top()->expire_time - time(nullptr);
      }
      if (wait_time > 0) {
        _cond_timer.wait_for(lock, std::chrono::milliseconds(wait_time));
      } else if (_timer_queue.empty()) {
        _cond_timer.wait(lock);
      }
      handle_timer();
    }
  });
}

Thread_clockr::~Thread_clockr() {
  if (_timer_queue.size() > 0)
    del_all_timer();
  if (_timer_thread.joinable()) {
    _timer_thread.join();
  }
}