
//用最小堆或者红黑树实现定时器
// stl中的priority_queue可以实现最小堆
// stl中的set map底层是红黑树
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
struct timer {
  int timer_id;                   //定时器id
  time_t expire_time;             //过期的时间戳
  std::function<void()> callback; //定时器回调函数
  int version{0};                 //是否是最新的定时器
  bool operator>(const timer &other) const {
    return expire_time > other.expire_time;
  }
  timer(int timer_id, time_t expire_time = 0,
        std::function<void()> callback = nullptr, int version = 0)
      : timer_id(timer_id), expire_time(expire_time), callback(callback),
        version(version) {}
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
public:
  //添加定时器//跟连接id一样，
  void add_timer(int timer_id, time_t without, std::function<void()> callback,
                 int version) {
    std::lock_guard<std::mutex> lock(_mutex);
    time_t expire_time = time(nullptr) + without;
    auto T = std::make_unique<timer>(timer_id, expire_time, callback, version);
    _timer_queue.push(std::move(T));
    _cond_timer.notify_one();
  }

  //删除所有定时器
  void del_all_timer() {
    std::lock_guard<std::mutex> lock(_mutex);
    while (!_timer_queue.empty()) {
      _timer_queue.pop();
    }
  }
  //修改定时器
  void mod_timer(int timer_id, time_t without, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _timer_map[timer_id]++;
    int new_version = _timer_map[timer_id];
    //添加新的定时器
    add_timer(timer_id, without, callback, new_version);
  }

  //处理快要到时间点的定时器
  void handle_timer() {
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
        continue;
      }
      if (_timer_queue.top()->callback) {
        _timer_queue.top()->callback();
      }
      _timer_queue.pop();
    }
  };

public:
  Thread_clockr() {
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
  ~Thread_clockr() {
    if (!_timer_queue.empty())
      del_all_timer();
  }
};
//底层是最小堆，那么让一个线程去做 每次添加定时器加锁
