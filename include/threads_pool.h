#pragma once
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
class pthreads {
private:
  std::mutex _queue_mutex;
  std::shared_mutex _queue_look_mutex;
  std::condition_variable _queue_cond;
  std::vector<std::thread> _threads;
  std::queue<std::function<void()>> _task_queue;
  std::atomic<bool> _stop;

public:
  pthreads(size_t threadCount = std::thread::hardware_concurrency())
      : _stop(false) {
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
  pthreads(const pthreads &) = delete;
  pthreads &operator=(const pthreads &) = delete;
  //提交任务到线程池，返回future,用于获取任务执行结果
  template <typename T, typename... Args>
  auto enqueue(T &&t, Args &&...args)
      -> std::future<typename std::invoke_result<T, Args...>> {
    using return_type = typename std::invoke_result_t<T, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<T>(t), std::forward<Args>(args)...));
    std::future<return_type> future_thread = task->get_future();
    {
      std::unique_lock<std::mutex> lock(_queue_mutex);
      if (_stop.load()) {
        throw std::runtime_error("pthreads is stopped");
      }

      _task_queue.emplace([task]() { (*task)(); });
    }
    _queue_cond.notify_one();
    return future_thread;
  }

  size_t pendingTasks() {
    std::shared_lock<std::shared_mutex> lock(_queue_look_mutex);
    return _task_queue
        .size(); // 注意：这个方法不是线程安全的，可能在返回时队列大小已改变
  }
  bool isRunning() const {
    return !_stop.load(); // 停止标志取反：true表示运行中，false表示已停止
  }
  void waitAll() {
    while (pendingTasks() > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  void shutdown() {
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
  ~pthreads() {
    if (!_stop.load()) {
      shutdown();
    }
  }
};