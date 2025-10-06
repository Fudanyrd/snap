#ifndef _THREAD_H
#define _THREAD_H

#define TPOOL_WORKERS 6

#ifndef TPOOL_TASKS
#define TPOOL_TASKS 24
#endif // TPOOL_TASKS

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>

#ifndef IAssert
#define IAssert(cond) assert(cond)
#endif

struct TaskNode {
  TaskNode *prev;
  TaskNode *next;

  void reset(void) {
    prev = next = (TaskNode *)0;
  }

  void remove(void) {
    TaskNode *p = this->prev;
    TaskNode *n = this->next;
    p->next = n;
    n->prev = p;
  }

  void insert(TaskNode *before) {
    TaskNode *p = before->prev;
    p->next = this;
    before->prev = this;
    this->prev = p;
    this->next = before;
  }
};

struct Task {
  TaskNode node;

  void (*routine)(void **);
  void *args[6];

  static Task *FromNode(TaskNode *node) {
    IAssert(node != (TaskNode *)0);
    uintptr_t ptr = (uintptr_t) node;
    ptr -= offsetof(struct Task, node);
    return (Task *)node;
  }
};

struct TaskQueue {
  TaskNode head;
  TaskNode tail;

  TaskQueue() {
    head.prev = tail.next = (TaskNode *)0;
    head.next = &tail;
    tail.prev = &head;
  }

  bool Empty() const {
    return head.next == &tail;
  }

  TaskNode *pop(void) {
    IAssert(!Empty());  
    TaskNode *node = head.next;
    node->remove();
    return node;
  }

  void push(Task *t) {
    TaskNode *node = &t->node;
    node->insert(&tail);
  }
};

class ThreadPool {
 private:
  pthread_t workers[TPOOL_WORKERS];

  pthread_cond_t worker_cond;
  pthread_mutex_t worker_lock;
  TaskQueue pending;
 
  /**
   * Used by host thread to track number of unclaimed tasks.
   * FIXME: this is only read/written by host thread. It does not
   * have to be lock-protected or atomic.
   */
  volatile int unclaimed;

  pthread_cond_t host_cond;
  pthread_mutex_t host_lock;
  TaskQueue completed;

  /**
   * Add the completed task to queue, and signal host thread.
   * Used by worker thread only.
   */
  void OnComplete(Task *completedTask);

  /**
   * Worker routine executed by worker thread.
   */
  static void *worker(void *);

  /**
   * (Used by worker thread) Get a task to execute.
   */
  Task *Execute(void);

 public:
  ThreadPool();
  
  /* Give exit_task to worker. */
  ~ThreadPool();

  /** 
   * Wait for any of the running task to finish.
   * @return the completed task, NULL if no executing task.
   */
  Task *waitFor(void);

  /**
   * Wait for all running task to complete.
   */
  void waitForAll(void) {
    while (this->waitFor() != (Task *)0) {
    }
  }

  /**
   * Add a task to the job queue, and wake up a 
   * sleeping worker.
   */
  void push(Task *task) {
    this->push(task, true);
  }

 private:
  void push(Task *task, bool check);
};

extern Task taskBuf[TPOOL_TASKS];
extern ThreadPool tpool;

#endif // _THREAD_H
