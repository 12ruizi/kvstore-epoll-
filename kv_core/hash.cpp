
#include "kvstore.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_KEY_LEN 128
#define MAX_VALUE_LEN 512
#define MAX_TABLE_SIZE 102400
#ifdef ENABLE_POINTER_KEY

typedef struct hashnode_s {
  char *key;
  char *value;
  struct hashnode_s *next;
} hashnode_t;

#else

typedef struct hashnode_s {
  char key[MAX_KEY_LEN];
  char value[MAX_VALUE_LEN];
  struct hashnode_s *next;
} hashnode_t;

#endif

typedef struct hashtable_s {
  hashnode_t **nodes;
  int max_slots;
  int count;
} hashtable_t;

hashtable_t Hash;
static int _hash(char *key, int size) {
  if (!key)
    return -1;

  int sum = 0;
  int i = 0;

  while (key[i] != 0) {
    sum += key[i];
    i++;
  }

  return sum % size;
}

hashnode_t *_create_node(char *key, char *value) {

  hashnode_t *node = (hashnode_t *)kvstore_malloc(sizeof(hashnode_t));
  if (!node)
    return NULL;

#if ENABLE_POINTER_KEY

  node->key = (char *)kvstore_malloc(strlen(key) + 1);
  if (!node->key) {
    kvstore_free(node);
    return NULL;
  }
  strcpy(node->key, key);

  node->value = (char *)kvstore_malloc(strlen(value) + 1);
  if (!node->value) {
    kvstore_free(node->key);
    kvstore_free(node);
    return NULL;
  }
  strcpy(node->value, value);

#else

  strncpy(node->key, key, MAX_KEY_LEN);
  strncpy(node->value, value, MAX_VALUE_LEN);

#endif

  node->next = NULL;

  return node;
}

//
int init_hashtable(hashtable_t *hash) {

  if (!hash)
    return -1;

  hash->nodes =
      (hashnode_t **)kvstore_malloc(sizeof(hashnode_t *) * MAX_TABLE_SIZE);
  if (!hash->nodes)
    return -1;

  hash->max_slots = MAX_TABLE_SIZE;
  hash->count = 0;

  return 0;
}

//
void dest_hashtable(hashtable_t *hash) {

  if (!hash)
    return;

  int i = 0;
  for (i = 0; i < hash->max_slots; i++) {
    hashnode_t *node = hash->nodes[i];

    while (node != NULL) { // error

      hashnode_t *tmp = node;
      node = node->next;
      hash->nodes[i] = node;

      kvstore_free(tmp);
    }
  }

  kvstore_free(hash->nodes);
}

// mp
int put_kv_hashtable(hashtable_t *hash, char *key, char *value) {

  if (!hash || !key || !value)
    return -1;

  int idx = _hash(key, MAX_TABLE_SIZE);

  hashnode_t *node = hash->nodes[idx];
#if 1
  while (node != NULL) {
    if (strcmp(node->key, key) == 0) { // exist
      return 1;
    }
    node = node->next;
  }
#endif

  hashnode_t *new_node = _create_node(key, value);
  new_node->next = hash->nodes[idx];
  hash->nodes[idx] = new_node;

  hash->count++;

  return 0;
}

char *get_kv_hashtable(hashtable_t *hash, char *key) {

  if (!hash || !key)
    return NULL;

  int idx = _hash(key, MAX_TABLE_SIZE);

  hashnode_t *node = hash->nodes[idx];

  while (node != NULL) {

    if (strcmp(node->key, key) == 0) {
      return node->value;
    }

    node = node->next;
  }

  return NULL;
}

int count_kv_hashtable(hashtable_t *hash) { return hash->count; }

int delete_kv_hashtable(hashtable_t *hash, char *key) {
  if (!hash || !key)
    return -2;

  int idx = _hash(key, MAX_TABLE_SIZE);

  hashnode_t *head = hash->nodes[idx];
  if (head == NULL)
    return -1; // noexist
  // head node
  if (strcmp(head->key, key) == 0) {
    hashnode_t *tmp = head->next;
    hash->nodes[idx] = tmp;

#if ENABLE_POINTER_KEY
    if (head->key) {
      kvstore_free(head->key);
    }
    if (head->value) {
      kvstore_free(head->value);
    }
    kvstore_free(head);
#else
    kvstore_free(head);
#endif
    hash->count--;

    return 0;
  }

  hashnode_t *cur = head;
  while (cur->next != NULL) {
    if (strcmp(cur->next->key, key) == 0)
      break; // search node

    cur = cur->next;
  }

  if (cur->next == NULL) {

    return -1;
  }

  hashnode_t *tmp = cur->next;
  cur->next = tmp->next;
#if ENABLE_POINTER_KEY
  if (tmp->key) {
    kvstore_free(tmp->key);
  }
  if (tmp->value) {
    kvstore_free(tmp->value);
  }
  kvstore_free(tmp);
#else
  kvstore_free(tmp);
#endif
  hash->count--;

  return 0;
}

int exist_kv_hashtable(hashtable_t *hash, char *key) {

  char *value = get_kv_hashtable(hash, key);
  if (value)
    return 1;
  else
    return 0;
}

// 5 + 2

int kvstore_hash_create(hashtable_t *hash) { return init_hashtable(hash); }

void kvstore_hash_destory(hashtable_t *hash) { return dest_hashtable(hash); }

int kvs_hash_set(hashtable_t *hash, char *key, char *value) {

  return put_kv_hashtable(hash, key, value);
}

char *kvs_hash_get(hashtable_t *hash, char *key) {

  return get_kv_hashtable(hash, key);
}

int kvs_hash_delete(hashtable_t *hash, char *key) {

  return delete_kv_hashtable(hash, key);
}

int kvs_hash_modify(hashtable_t *hash, char *key, char *value) {

  if (!hash || !key || !value)
    return -1;

  int idx = _hash(key, MAX_TABLE_SIZE);

  hashnode_t *node = hash->nodes[idx];

  while (node != NULL) {

    if (strcmp(node->key, key) == 0) {
      kvstore_free(node->value);

      node->value = (char *)kvstore_malloc(strlen(value) + 1);
      if (node->value) {
        strcpy(node->value, value);
        return 0;
      } else
        assert(0);
    }

    node = node->next;
  }

  return -1;
}

int kvs_hash_count(hashtable_t *hash) { return hash->count; }
// Save hash table to disk
int save_to_disk(hashtable_t *hash, const char *filename) {
  if (!hash || !filename) {
    printf("save_to_disk: Invalid parameters\n");
    return -1;
  }

  printf("save_to_disk: Attempting to save to file '%s'\n", filename);
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    printf("save_to_disk: Error opening file '%s' for writing\n", filename);
    perror("perror");
    return -1;
  }

  int count = 0;
  // Iterate through all buckets in the hash table
  for (int i = 0; i < hash->max_slots; i++) {
    hashnode_t *node = hash->nodes[i];
    while (node) {
      // Write key-value pair to file
      fprintf(fp, "%s:%s\n", node->key, node->value);
      node = node->next;
      count++;
    }
  }

  printf("save_to_disk: Successfully saved %d key-value pairs to '%s'\n", count,
         filename);
  fclose(fp);
  return 0;
}

// Load hash table from disk
int load_from_disk(hashtable_t *hash, const char *filename) {
  if (!hash || !filename) {
    printf("load_from_disk: Invalid parameters\n");
    return -1;
  }

  printf("load_from_disk: Attempting to load from file '%s'\n", filename);

  // Check if file exists
  if (access(filename, F_OK) == -1) {
    // File doesn't exist, that's OK
    printf("load_from_disk: File '%s' does not exist, skipping load\n",
           filename);
    return 0;
  }

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("load_from_disk: Error opening file '%s' for reading\n", filename);
    perror("perror");
    return -1;
  }

  int count = 0;
  char line[BUFFER_LENGTH];
  while (fgets(line, sizeof(line), fp)) {
    // Remove trailing newline
    line[strcspn(line, "\n")] = 0;

    // Find the delimiter
    char *delimiter = strchr(line, ':');
    if (delimiter) {
      *delimiter = '\0';
      char *key = line;
      char *value = delimiter + 1;

      // Insert into hash table
      int result = put_kv_hashtable(hash, key, value);
      if (result >= 0) {
        count++;
      }
    }
  }

  printf("load_from_disk: Successfully loaded %d key-value pairs from '%s'\n",
         count, filename);
  fclose(fp);
  return 0;
}