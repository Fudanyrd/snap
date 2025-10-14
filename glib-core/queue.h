#ifndef _QUEUE_H
#define _QUEUE_H
#include "thread.h"

template <typename _Tp>
struct ConcurrentQueueNode {
  ConcurrentQueueNode<_Tp> *next;
  _Tp item;
  
  ConcurrentQueueNode() : next(0), item() {}
};

template <typename _Tp>
class ConcurrentQueue {
 private:
   ConcurrentQueueNode<_Tp> dummy;
   ConcurrentQueueNode<_Tp> *tail;
   pthread_spinlock_t qlock;
 public:
  ConcurrentQueue() : dummy() {
    pthread_spin_init(&qlock, PTHREAD_PROCESS_PRIVATE);
    tail = &dummy;
  }
#if __cplusplus >= 201103L
  ~ConcurrentQueue() = default;
#endif

  void push(ConcurrentQueueNode<_Tp> *node);
  ConcurrentQueueNode<_Tp> *pop(void);

  /**
   * Append the nodes of `other`, and clear `other`.
   * @return true if `other` is not empty.
   */
  bool append(ConcurrentQueue<_Tp> &other);
};

template <typename _Tp>
void ConcurrentQueue<_Tp>::push(ConcurrentQueueNode<_Tp> *node) {
  node->next = (ConcurrentQueueNode<_Tp> *)0;
  pthread_spin_lock(&qlock);
  tail->next = node;
  tail = node;
  pthread_spin_unlock(&qlock);
}

template <typename _Tp>
ConcurrentQueueNode<_Tp> *ConcurrentQueue<_Tp>::pop(void) {
  ConcurrentQueueNode<_Tp> *node;
  ConcurrentQueueNode<_Tp> *dummyPt = &this->dummy;
  pthread_spin_lock(&this->qlock);
  node = dummyPt->next;
  if (node == (ConcurrentQueueNode<_Tp> *) 0) {
    tail = dummyPt;
    pthread_spin_unlock(&this->qlock);
    return node;
  }
  dummyPt->next = node->next;
  pthread_spin_unlock(&this->qlock);
  /* Clear. */
  node->next = (ConcurrentQueueNode<_Tp> *) 0;
  return node;
}

template <typename _Tp>
bool ConcurrentQueue<_Tp>::append(ConcurrentQueue<_Tp> &other) {
  ConcurrentQueueNode<_Tp> *first = other.dummy.next;
  if (first == 0) {
    return false;
  }

  this->tail->next = first;
  this->tail = other.tail;

  /* Clear other's content. */
  other.dummy.next = 0;
  other.tail = &other.dummy;

  return true;
}

#endif // _QUEUE_H
