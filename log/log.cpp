#include "log.h"
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
kv_log::kv_log() { _used_lines = 0; }

kv_log::~kv_log() {
  if (_fp) {
    fclose(_fp);
  }
}

bool kv_log::init(const char *file_name, int log_buffer_size, int split_lines) {
  std::lock_guard<std::mutex> lock(log_mutex);
  _max_log_buffer_size = log_buffer_size;
  _max_log_lines = split_lines;
  _buffer = new char[_max_log_buffer_size];
  memset(_buffer, '\0', _max_log_buffer_size);
  time_t t = time(NULL);
  struct tm *sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;

  const char *p = strrchr(file_name, '/');
  char log_file_name[256] = {0};

  //定义日志文件名字
  if (p == NULL) {
    snprintf(log_file_name, 255, "%0d_%02d_%02d_%s", my_tm.tm_year + 1900,
             my_tm.tm_mon + 1, my_tm.tm_mday, file_name);

  } else {
    strcpy(_log_name, p + 1); // p指向的/  p+1指向文件名首地址
    strncpy(_dir_name, file_name, p - file_name + 1);
    snprintf(log_file_name, 255, "%s%d_%02d_%02d_%s", _dir_name,
             my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, _log_name);
  }
  _today = my_tm.tm_mday;
  _fp = fopen(log_file_name, "a");
  if (_fp == NULL) {
    return false;
  }

  return true;
}
void kv_log::write_log(int level, const char *format, ...) {
  if (!_fp) {
    perror("fopen log file");
    return;
  }
  struct timeval now = {0, 0};
  gettimeofday(&now, NULL);
  time_t t = now.tv_sec;
  struct tm *sys_tm = localtime(&t);
  struct tm my_tm = *sys_tm;
  char lable[16] = {0};

  switch (level) {
  case 0: {
    strcpy(lable, "[debug]: ");
    break;
  }
  case 1: {
    strcpy(lable, "[info]: ");
    break;
  }

  case 2: {
    strcpy(lable, "[warning]: ");
    break;
  }
  case 3: {
    strcpy(lable, "[error]: ");
    break;
  }

  default: {
    strcpy(lable, "[info]: ");
    break;
  }
  }
  std::lock_guard<std::mutex> lock(log_mutex);
  {

    _used_lines++;
    if (_today != my_tm.tm_mday || _used_lines % _max_log_lines == 0) {
      //说明行数到了最大行数，需要换文件，日期变换更为新的文件；
      char new_log[256] = {0};

      fflush(_fp);
      fclose(_fp);
      char tail[16] = {0};
      snprintf(tail, 16, "%d_%02d_%02d", my_tm.tm_year + 1900, my_tm.tm_mon + 1,
               my_tm.tm_mday);
      if (_today != my_tm.tm_mday) {
        snprintf(new_log, 255, "%s%s%s", _dir_name, tail, _log_name);
      } else {
        snprintf(new_log, 255, "%s%s%s.%lld", _dir_name, tail, _log_name,
                 _used_lines / _max_log_lines);
      }
      _fp = fopen(new_log, "a");
      if (_fp == NULL) {
        perror("fopen log file");
        return;
      }
      va_list valist;
      va_start(valist, format);
      std::string log_str;

      int n =
          snprintf(_buffer, 48, "%d-%02d_%02d %02d:%02d:%02d.%06ld %s",
                   my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                   my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_sec, lable

          );

      int m = vsnprintf(_buffer + n, _max_log_buffer_size - 1, format, valist);

      _buffer[n + m] = '\n';
      _buffer[n + m + 1] = '\0';
      log_str = _buffer;

#ifdef ASYNC_LOG
      {

      }
#else
      { fputs(log_str.c_str(), _fp); }
#endif
      va_end(valist);
    }
  }
}
