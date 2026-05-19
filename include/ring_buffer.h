
//设计一个用户态环形缓冲区
// 1考虑缓冲区大小，定长
#define RING_BUFFER_SIZE 1024
class ring_buffer {
private:
  char _buffer[RING_BUFFER_SIZE];
  int _head;
  int _tail;

public:
  ring_buffer() : _head(0), _tail(0){};
  ~ring_buffer() = default;
  char *get_buffer() { return _buffer; }
  int get_head() const { return _head; }
  int get_tail() const { return _tail; }
  //只需要负责更新 head和tail

  int not_useSize() {
    int used_szie;
    if (_tail >= _head) {
      used_szie = _tail - _head;

    } else {
      used_szie = RING_BUFFER_SIZE - _head + _tail;
    }
    return RING_BUFFER_SIZE - used_szie;
  } // return ring_buffer_size -（tail -
    // head+ring_buffer_size）%ring_buffer_size
  int used_size() {
    return (_tail - _head + RING_BUFFER_SIZE) % RING_BUFFER_SIZE;
  }
  bool writed(int len) {
    if (not_useSize() < len) {
      return false;
    }
    //更新tail
    _tail = (_tail + len) % RING_BUFFER_SIZE;
    return true;
  }
  bool readed(int len) {
    if (_head == _tail) {
      return false;
    } //更新head
    _head = (_head + len) % RING_BUFFER_SIZE;
    return true;
  }
};