#pragma once
#include <mutex>
#include <stdio.h>
#include <string>
class kv_log {
private:
  kv_log();
  virtual ~kv_log();
  char _dir_name[128];
  char _log_name[128];
  int _max_log_lines;
  int _max_log_buffer_size;
  long long _used_lines;
  int _today;
  FILE *_fp;
  char *_buffer;
  std::mutex log_mutex;

public:
  static kv_log *get_instance() {
    static kv_log instance;
    return &instance;
  }
  bool init(const char *file_name, int log_buffer_size = 8192,
            int split_lines = 1000000);

  static void *flush_log_thread(void *arg) {
    kv_log::get_instance()->flush_log();
    return nullptr;
  }
  //写日志
  void write_log(int level, const char *format, ...);
  //刷新日志
  void flush_log();
};