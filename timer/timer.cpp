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
int Thread_clockr::add_timer(int timer_id, time_t without,
                             std::function<void()> callback, int version) {
  std::lock_guard<std::mutex> lock(_mutex);
  time_t expire_time = time(nullptr) + without;
  auto T = std::make_unique<timer>(timer_id, expire_time, callback, version);
  _timer_queue.push(std::move(T));
  _timer_map[timer_id] = version;
  _cond_timer.notify_one();
  return timer_id;
}

//删除所有定时器
bool Thread_clockr::del_all_timer() {
  std::lock_guard<std::mutex> lock(_mutex);
  while (!_timer_queue.empty()) {
    _timer_queue.pop();
  }
  return true;
}

//修改定时器
bool Thread_clockr::mod_timer(int timer_id, time_t without,
                              std::function<void()> callback) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _timer_map[timer_id]++;
  }
  int new_version = _timer_map[timer_id];
  //添加新的定时器
  add_timer(timer_id, without, callback, new_version);
  return true;
}

//处理快要到时间点的定时器
void Thread_clockr::handle_timer() {
  {
    std::vector<std::function<void()>> callbacks;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      time_t now = time(nullptr);
      while (!_timer_queue.empty() && _timer_queue.top()->expire_time <= now) {
        auto &top = _timer_queue.top();
        if (_timer_map.count(top->timer_id) &&
            _timer_map[top->timer_id] != top->version) {
          std::cout << "跳过旧版本" << std::endl;
          //释放timer

          _timer_queue.pop(); // 跳过旧版本
          continue;
        }
        if (top->version == 0) {
          _timer_map.erase(top->timer_id);
          _timer_queue.pop();
          continue;
        }
        if (top->callback) {
          callbacks.push_back(top->callback);
          std::cout << "callback success" << std::endl;
        }
        _timer_queue.pop();
      }
    }
    for (auto &callback : callbacks) {
      callback();
    }
  }
}

Thread_clockr::Thread_clockr() : _stop_flag(false) {
  _timer_thread = std::thread([this]() {
    while (!_stop_flag) { // 修改为检查停止标志
      std::cout << "thread_clockr handle_timer" << std::endl;
      std::unique_lock<std::mutex> lock(_mutex);
      //查看堆顶的时间戳和现在的时间戳计算等待时间
      int wait_time = 0;
      if (!_timer_queue.empty() && !_stop_flag) { // 添加停止标志检查
        wait_time = _timer_queue.top()->expire_time - time(nullptr);
      }
      lock.unlock();
      if (!_stop_flag) { // 只有在未停止时才等待
        if (wait_time > 0) {
          std::cout << "wait_time_cond: " << wait_time << std::endl;
          // 使用带超时的条件变量等待，这样可以在等待期间响应停止信号
          std::unique_lock<std::mutex> lock2(_mutex);
          _cond_timer.wait_for(lock2, std::chrono::seconds(wait_time),
                               [this]() { return _stop_flag; });
        } else if (_timer_queue.empty()) {
          std::unique_lock<std::mutex> lock2(_mutex);
          // 使用条件变量的谓词等待，当_stop_flag为true时立即返回
          _cond_timer.wait(
              lock2, [this]() { return _stop_flag || !_timer_queue.empty(); });
        }
      }

      if (!_stop_flag) {
        handle_timer();
      }
    }
  });
}

Thread_clockr::~Thread_clockr() {
  if (_timer_queue.size() > 0)
    del_all_timer();

  {
    std::lock_guard<std::mutex> lock(_mutex);
    _stop_flag = true; // 设置停止标志
  }
  _cond_timer.notify_all(); // 唤醒等待的线程

  if (_timer_thread.joinable()) {
    _timer_thread.join();
  }
}

bool Thread_clockr::del_timer(int timer_id) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_timer_map.count(timer_id)) {
    //标记删除timer，将版本置为0
    _timer_map[timer_id] = 0;
    _cond_timer.notify_one();
    return true;
  }
  return false;
}