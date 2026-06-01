
#ifndef MEMORY_H
#define MEMORY_H

#include "../network/connect_info/connect_info.h"
#include "../network/protocol_parser/protocol_parser.h"
#include <map>
#include <memory>
#include <string>
//固定大小的内存chunk
class Conn_pool {
private:
  union conn_block {
    conn_block *next;
    char data[sizeof(ConnectionInfo)];
    conn_block() { next = nullptr; }
  };
  struct conn_pool_control {
    size_t _total_blocks;
    size_t _block_size;
    char *memory_head;     //内存的头指针
    conn_block *free_list; //空闲链表头指针
    std::unique_ptr<std::map<size_t, ConnectionInfo *>> _map; //块映射表
    conn_pool_control(size_t total_blocks, size_t block_size) {
      _total_blocks = total_blocks;
      _block_size = block_size;
      free_list = nullptr;
      _map = std::make_unique<std::map<size_t, ConnectionInfo *>>();
      memory_head = nullptr;
    }
  };

  std::unique_ptr<conn_pool_control> _control;

public:
  Conn_pool(size_t total_blocks = 10240, size_t block_size = 1024) {
    _control = std::make_unique<conn_pool_control>(total_blocks, block_size);
    _control->_block_size = block_size;
    _control->_total_blocks = total_blocks;
    _control->memory_head = (char *)malloc(total_blocks * block_size);
    if (_control->memory_head == nullptr) {
      return;
    }
    //初始化空闲链表
    _control->free_list = (conn_block *)_control->memory_head;
    conn_block *current = _control->free_list;
    for (size_t i = 0; i < _control->_total_blocks - 1; i++) {
      current->next = (conn_block *)((char *)current + _control->_block_size);
      current = current->next;
    }
    current->next = nullptr;
  }
  //获取连接信息
  ConnectionInfo *acquire() {
    if (_control->free_list == nullptr) {
      return nullptr;
    }
    conn_block *block = _control->free_list;
    _control->free_list = block->next;

    return new (block->data) ConnectionInfo(); // new定位构造
  }
  //初始化完成后将信息存放到 map里面
  void add_map(size_t id, ConnectionInfo *info) {
    _control->_map->insert({id, info});
  }
  //根据id获取连接信息
  ConnectionInfo *get(size_t id) { return _control->_map->find(id)->second; }

  //回收对象
  void Release(ConnectionInfo *info) {
    //调用析构函数
    info->~ConnectionInfo();
    conn_block *block = (conn_block *)&info;
    block->next = _control->free_list;
    _control->free_list = block;
  }

  ~Conn_pool() {
    if (_control->memory_head) {
      char *block_start = _control->memory_head;
      for (size_t i = 0; i < _control->_total_blocks; ++i) {
        void *current_block_addr = block_start + (i * _control->_block_size);
        ConnectionInfo *info =
            reinterpret_cast<ConnectionInfo *>(current_block_addr);
        info->~ConnectionInfo();
      }

      // 2. 真正释放底层的物理内存
      free(_control->memory_head);
      _control->memory_head = nullptr;
    }
  }
};
#endif