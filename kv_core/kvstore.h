

#ifndef __KVSTORE_H__
#define __KVSTORE_H__
#include "../log/Log.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_LENGTH 512
#define ENABLE_LOG 1         //日志开关
#define ENABLE_POINTER_KEY 1 // 启用指针键值存储以支持持久化
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

// Persistence functions
int save_to_disk(hashtable_t *hash, const char *filename);
int load_from_disk(hashtable_t *hash, const char *filename);

#endif // __KVSTORE_H_