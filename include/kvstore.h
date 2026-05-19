#ifndef __KVSTORE_H__
#define __KVSTORE_H__
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

typedef int (*RCALLBACK)(int fd);
struct conn_item {
  int fd;                      /**< 客户端文件描述符 */
  char rbuffer[BUFFER_LENGTH]; /**< 读缓冲区 */
  int rlen;                    /**< 读缓冲区数据长度 */
  char wbuffer[BUFFER_LENGTH]; /**< 写缓冲区 */
  int wlen;                    /**< 写缓冲区数据长度 */
  union {
    RCALLBACK accept_callback; /**< 接受连接回调 */
    RCALLBACK recv_callback;   /**< 接收数据回调 */
  } recv_t;                    /**< 接收相关回调函数 */
  RCALLBACK send_callback;     /**< 发送数据回调函数 */
};
int epoll_entry(void);
int kvstore_request(struct conn_item *item);
void *kvstore_malloc(size_t size);
void kvstore_free(void *ptr);
typedef struct hashtable_s hashtable_t;

extern hashtable_t Hash;

int kvstore_hash_create(hashtable_t *hash);
void kvstore_hash_destory(hashtable_t *hash);
int kvs_hash_set(hashtable_t *hash, char *key, char *value);
char *kvs_hash_get(hashtable_t *hash, char *key);
int kvs_hash_delete(hashtable_t *hash, char *key);
int kvs_hash_modify(hashtable_t *hash, char *key, char *value);
int kvs_hash_count(hashtable_t *hash);

#endif // __KVSTORE_H_