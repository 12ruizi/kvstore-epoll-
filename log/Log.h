#pragma once
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string>
#define SPLIT_LINES 1000000
#define LOG_BUFFER_SIZE 1024
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

#define LOG_INFO(format, ...)                                                  \
  if (LOG_FLAG) {                                                              \
    kv_log::get_instance()->write_log(1, format, ##__VA_ARGS__);               \
    kv_log::get_instance()->flush_log();                                       \
  }
#define LOG_DEBUG(format, ...)                                                 \
  if (LOG_FLAG) {                                                              \
    kv_log::get_instance()->write_log(0, format, ##__VA_ARGS__);               \
    kv_log::get_instance()->flush_log();                                       \
  }

#define LOG_WARN(format, ...)                                                  \
  if (LOG_FLAG) {                                                              \
    kv_log::get_instance()->write_log(2, format, ##__VA_ARGS__);               \
    kv_log::get_instance()->flush_log();                                       \
  }
#define LOG_ERROR(format, ...)                                                 \
  if (LOG_FLAG) {                                                              \
    kv_log::get_instance()->write_log(3, format, ##__VA_ARGS__);               \
    kv_log::get_instance()->flush_log();                                       \
  }
