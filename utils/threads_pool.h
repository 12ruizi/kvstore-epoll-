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
  pthreads(size_t threadCount = std::thread::hardware_concurrency());
  pthreads(const pthreads &) = delete;
  pthreads &operator=(const pthreads &) = delete;
  //提交任务到线程池，返回future,用于获取任务执行结果
  template <typename T, typename... Args>
  auto enqueue(T &&t, Args &&...args)
      -> std::future<std::invoke_result_t<T, Args...>> {
    using return_type = std::invoke_result_t<T, Args...>;
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

  size_t pendingTasks();
  bool isRunning() const;
  void waitAll();
  void shutdown();
  ~pthreads();
};