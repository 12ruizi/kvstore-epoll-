#include "kvstore.h"
#define KVSTORE_MAX_TOKENS 128
const char *commands[] = {
    "SET",  "GET",    "DEL",  "MOD",  "COUNT", "RSET", "RGET",   "RDEL",
    "RMOD", "RCOUNT", "HSET", "HGET", "HDEL",  "HMOD", "HCOUNT",
};
enum {
  KVS_CMD_START = 0,
  KVS_CMD_SET = KVS_CMD_START,
  KVS_CMD_GET,
  KVS_CMD_DEL,
  KVS_CMD_MOD,
  KVS_CMD_COUNT,

  KVS_CMD_RSET,
  KVS_CMD_RGET,
  KVS_CMD_RDEL,
  KVS_CMD_RMOD,
  KVS_CMD_RCOUNT,

  KVS_CMD_HSET,
  KVS_CMD_HGET,
  KVS_CMD_HDEL,
  KVS_CMD_HMOD,
  KVS_CMD_HCOUNT,

  KVS_CMD_SIZE,
};
void *kvstore_malloc(size_t size) {
#if ENABLE_MEM_POOL
#else
  return malloc(size);
#endif
}
void kvstore_free(void *ptr) {
#if ENABLE_MEM_POOL
#else
  return free(ptr);
#endif
}
int kvstore_hash_set(char *key, char *value) {
  return kvs_hash_set(&Hash, key, value);
}
char *kvstore_hash_get(char *key) { return kvs_hash_get(&Hash, key); }
int kvstore_hash_delete(char *key) { return kvs_hash_delete(&Hash, key); }
int kvstore_hash_modify(char *key, char *value) {
  return kvs_hash_modify(&Hash, key, value);
}
int kvstore_hash_count(void) { return kvs_hash_count(&Hash); }

int kvstore_split_token(char *msg, char **tokens) {

  if (msg == NULL || tokens == NULL)
    return -1;

  int idx = 0;
  char *token = strtok(msg, " ");
  while (token != NULL) {
    tokens[idx++] = token;
    token = strtok(NULL, " ");
  }
  return idx;
}

int kvstore_parser_protocol(char *wmsg, char **tokens, int count) {

  if (wmsg == NULL || tokens[0] == NULL || count == 0)

  {
    return -1;
  }

  int cmd = KVS_CMD_START;

  for (cmd = KVS_CMD_START; cmd < KVS_CMD_SIZE; cmd++) {
    if (strcmp(commands[cmd], tokens[0]) == 0) {
      break;
    }
  }
  char *key = tokens[1];
  char *value = tokens[2];
  memset(wmsg, 0, BUFFER_LENGTH);
  switch (cmd) {
  // hash
  case KVS_CMD_HGET: {

    char *val = kvstore_hash_get(key);
    if (val) {
      snprintf(wmsg, BUFFER_LENGTH, "%s", val);
    } else {
      snprintf(wmsg, BUFFER_LENGTH, "NO EXIST");
    }

    break;
  }
  case KVS_CMD_HDEL: {

    int res = kvstore_hash_delete(key);
    if (res < 0) { // server
      snprintf(wmsg, BUFFER_LENGTH, "%s", "ERROR");
    } else if (res == 0) {
      snprintf(wmsg, BUFFER_LENGTH, "%s", "SUCCESS");
    } else {
      snprintf(wmsg, BUFFER_LENGTH, "NO EXIST");
    }

    break;
  }
  case KVS_CMD_HMOD: {

    int res = kvstore_hash_modify(key, value);
    if (res < 0) { // server
      snprintf(wmsg, BUFFER_LENGTH, "%s", "ERROR");
    } else if (res == 0) {
      snprintf(wmsg, BUFFER_LENGTH, "%s", "SUCCESS");
    } else {
      snprintf(wmsg, BUFFER_LENGTH, "NO EXIST");
    }

    break;
  }

  case KVS_CMD_HCOUNT: {
    int count = kvstore_hash_count();
    if (count < 0) { // server
      snprintf(wmsg, BUFFER_LENGTH, "%s", "ERROR");
    } else {
      snprintf(wmsg, BUFFER_LENGTH, "%d", count);
    }
    break;
  }

  default: {
    printf("cmd: %s\n", commands[cmd]);
    // assert(0);//直接崩溃
    snprintf(wmsg, BUFFER_LENGTH, "ERROR");
    break;
  }
  }
}

int kvstore_request(char *rmsg, char *wmsg) {
  char *tokens[KVSTORE_MAX_TOKENS];
  //把msg按空格分割成tokens数组
  int count = kvstore_split_token(rmsg, tokens);
  //解析处理tokens数组
  kvstore_parser_protocol(wmsg, tokens, count);
  return 0;
}

void init_kvengine(void) { kvstore_hash_create(&Hash); }

void exit_kvengine(void) { kvstore_hash_destory(&Hash); }
