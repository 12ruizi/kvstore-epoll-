#include "threads_pool.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <stdio.h>
#include <thread>
#include <vector>

pthreads::pthreads(size_t threadCount) : _stop(false) {
  if (threadCount == 0) {
    threadCount = 4;
  }
  for (size_t i = 0; i < threadCount; i++) {
    _threads.emplace_back([this]() {
      while (true) {
        std::function<void()> task; //创建函数
        {
          std::unique_lock<std::mutex> lock(_queue_mutex);
          this->_queue_cond.wait(lock, [this] {
            return this->_stop.load() || !this->_task_queue.empty();
          });
          if (this->_stop.load() && this->_task_queue.empty()) {
            return;
          }
          task = std::move(this->_task_queue.front());
          this->_task_queue.pop();
        }
        task();
      }
    });
  }
}

size_t pthreads::pendingTasks() {
  std::shared_lock<std::shared_mutex> lock(_queue_look_mutex);
  return _task_queue
      .size(); // 注意：这个方法不是线程安全的，可能在返回时队列大小已改变
}

bool pthreads::isRunning() const {
  return !_stop.load(); // 停止标志取反：true表示运行中，false表示已停止
}

void pthreads::waitAll() {
  while (pendingTasks() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void pthreads::shutdown() {
  {
    std::unique_lock<std::mutex> lock(_queue_mutex); // 加锁并设置退出标志
    _stop = true;
  } // 括号的作用是解锁
  _queue_cond.notify_all();
  for (std::thread &worker : _threads) {
    if (worker.joinable()) { // 检查线程是否可以join
      worker.join();
    }
  }
}

pthreads::~pthreads() {
  if (!_stop.load()) {
    shutdown();
  }
}