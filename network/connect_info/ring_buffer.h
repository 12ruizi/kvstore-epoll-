//设计一个用户态环形缓冲区
// 1考虑缓冲区大小，定长
#define RING_BUFFER_SIZE 1024
class ring_buffer {
private:
  char _buffer[RING_BUFFER_SIZE];
  int _head;
  int _tail;

public:
  ring_buffer();
  ~ring_buffer() = default;
  char *get_buffer();
  int get_head() const;
  int get_tail() const;
  //只需要负责更新 head和tail

  int not_useSize();
  int used_size();
  bool writed(int len);
  bool readed(int len);
};