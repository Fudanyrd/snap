/** 
 * A ring buffer that can cocurrently pop or push items, but not
 * both! For example, when executing BFS concurrently, all threads
 * only pop nodes from one ring buffer, and push new nodes to another.
 */
#ifndef _RING_BUF_H
#define _RING_BUF_H

#include <bits/move.h>
#include <cstddef>

template <typename _Tp>
class RingBuf {
 public:
  RingBuf(size_t len) {
    buf = new _Tp[len];
    vol = len;
    head = tail = 0;
  }
  ~RingBuf() { delete[] this->buf; }
  
  _Tp *data() { return this->buf; }
  const _Tp *data() const { return this->buf; }

  /* Pop an item from head. */
  _Tp *pop(void) {
    size_t idx = __atomic_fetch_add(&head, 1, __ATOMIC_ACQUIRE);
    if (idx >= tail) {
      return NULL;
    }

    return buf + idx;
  }

  void push(const _Tp &v) {
    size_t idx = __atomic_fetch_add(&tail, 1, __ATOMIC_ACQUIRE);
    /* idx %= vol; */
    buf[idx] = v;
  }

  void reset() {
    head = tail = 0;
  }

  size_t size() const {
    return this->tail;
  }

  bool empty() const {
    return size() == 0;
  }

  static void exchange(RingBuf &a, RingBuf &b) {
    std::swap(a.buf, b.buf);
    std::swap(a.head, b.head);
    std::swap(a.tail, b.tail);
    std::swap(a.vol, b.vol);
  }

 private:
  volatile size_t head;
  volatile size_t tail;
  size_t vol; /* Immutable. */
  _Tp *buf;
};

#endif  // _RING_BUF_H
