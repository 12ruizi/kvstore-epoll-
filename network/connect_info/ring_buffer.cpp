#include "ring_buffer.h"
#include <cstring>

ring_buffer::ring_buffer() : _head(0), _tail(0) {}

char *ring_buffer::get_buffer() { return _buffer; }

int ring_buffer::get_head() const { return _head; }

int ring_buffer::get_tail() const { return _tail; }

//只需要负责更新 head和tail
int ring_buffer::not_useSize() {
  int used_szie;
  if (_tail >= _head) {
    used_szie = _tail - _head;
  } else {
    used_szie = RING_BUFFER_SIZE - _head + _tail;
  }
  return RING_BUFFER_SIZE - used_szie;
} // return ring_buffer_size -（tail - head+ring_buffer_size）%ring_buffer_size

int ring_buffer::used_size() {
  return (_tail - _head + RING_BUFFER_SIZE) % RING_BUFFER_SIZE;
}

bool ring_buffer::writed(int len) {
  if (not_useSize() < len) {
    return false;
  }
  //更新tail
  _tail = (_tail + len) % RING_BUFFER_SIZE;
  return true;
}

bool ring_buffer::readed(int len) {
  if (_head == _tail) {
    return false;
  } //更新head
  _head = (_head + len) % RING_BUFFER_SIZE;
  return true;
}