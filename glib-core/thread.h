#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>

#define TPOOL_WORKERS 8

struct Task {
  void *args[6];
  void (*routine)(void **);
  
  /**
   * The worker thread will broadcast this condvar
   * When the routine finishes.
   */
  pthread_cond_t host_cond;

  pthread_mutex_t lock;
  /** Track number of completed workers. */
  volatile int finished;

  Task() : finished(0) {
    pthread_cond_init(&host_cond, (const pthread_condattr_t *) 0);
    pthread_mutex_init(&lock, (const pthread_mutexattr_t *) 0);
  }

  void waitfor(void);
};

class ThreadPool {
  pthread_t workers[TPOOL_WORKERS];

  /** Used to notify workers of incoming tasks. */
  pthread_cond_t cond;

  pthread_mutex_t lock;
  Task *tasks[TPOOL_WORKERS];
  int nextSlot;

 public:
  /** 
   * @param nt: number of threads to spawn.
   */
  ThreadPool(void);
  ~ThreadPool();

  Task *Execute(void);

  void AddTasks(Task *, int);
};

#endif // THREAD_H
