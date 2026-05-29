#ifndef __KVSTORE_H__
#define __KVSTORE_H__
#include "../include/connect_info.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_LENGTH 512
#define ENABLE_LOG 1 //日志开关
#ifdef ENABLE_LOG
#define LOG(_fmt, ...)                                                         \
  fprintf(stdout, "[%s:%d]: %s"_fmt, __FILE__, __LINE__, __VAR_ARGS__)
#else

#define LOG(_fmt, ...) // 日志输出宏，如果日志开关打开，则输出日志，否则不输出
                       // 输出格式：[文件名:行号]: 日志内容
#endif
int epoll_entry(void);
int kvstore_request(char *rmsg, char *wmsg);
void *kvstore_malloc(size_t size);
void kvstore_free(void *ptr);
void exit_kvengine(void);
void init_kvengine(void);
typedef struct hashtable_s hashtable_t;

extern hashtable_t Hash; // c++用单例模式实现

int kvstore_hash_create(hashtable_t *hash);
void kvstore_hash_destory(hashtable_t *hash);
int kvs_hash_set(hashtable_t *hash, char *key, char *value);
char *kvs_hash_get(hashtable_t *hash, char *key);
int kvs_hash_delete(hashtable_t *hash, char *key);
int kvs_hash_modify(hashtable_t *hash, char *key, char *value);
int kvs_hash_count(hashtable_t *hash);

#endif // __KVSTORE_H_