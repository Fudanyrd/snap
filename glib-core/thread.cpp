#include <string.h>

#include "thread.h"

void *ThreadPool::worker(void *pool) {

  ThreadPool *tpool = (ThreadPool *)pool;
  for (;;) {
    Task *task = tpool->Execute();
    task->node.reset();
    if (!task->routine) {
      /* Be asked to exit. */
      break;
    }

    task->routine(task->args);

    tpool->OnComplete(task);
  }

  pthread_exit((void *) 0);
}

void ThreadPool::OnComplete(Task *completedTask) {
  pthread_mutex_lock(&this->host_lock);
  this->completed.push(completedTask);
  pthread_mutex_unlock(&this->host_lock);
  pthread_cond_signal(&this->host_cond);
}

Task *ThreadPool::Execute(void) {
  TaskNode *node;
  pthread_mutex_lock(&this->worker_lock);
  while (pending.Empty()) {
    pthread_cond_wait(&this->worker_cond, &this->worker_lock);
  }
  node = pending.pop();
  pthread_mutex_unlock(&this->worker_lock);

  return Task::FromNode(node);
}

void ThreadPool::push(Task *task, bool check) {
  if (check)
    IAssert (task && task->routine);
  __atomic_fetch_add(&this->unclaimed, 1, __ATOMIC_RELAXED);
  pthread_mutex_lock(&this->worker_lock);
  this->pending.push(task);
  pthread_mutex_unlock(&this->worker_lock);
  pthread_cond_signal(&this->worker_cond);
}

Task *ThreadPool::waitFor(void) {
  IAssert(this->unclaimed >= 0);

  if (this->unclaimed == 0) {
    return (Task *)0;
  }

  pthread_mutex_lock(&this->host_lock);
  while (this->completed.Empty()) {
    pthread_cond_wait(&this->host_cond, &this->host_lock);
  }
  TaskNode *node = this->completed.pop();
  pthread_mutex_unlock(&this->host_lock);
  __atomic_fetch_sub(&this->unclaimed, 1, __ATOMIC_RELAXED);

  return Task::FromNode(node);
}

ThreadPool::~ThreadPool() {
  while (this->waitFor() != (Task *)0) {}

  for (int i = 0; i < TPOOL_WORKERS; i++) {
    Task *t = &taskBuf[i];    
    memset(t, 0, sizeof(*t));
    this->push(t, false);
  }

  for (int i = 0; i < TPOOL_WORKERS; i++) {
    pthread_join(this->workers[i], NULL);
  }
}

Task taskBuf[TPOOL_TASKS];
ThreadPool tpool;

ThreadPool::ThreadPool() {
  this->host_cond = PTHREAD_COND_INITIALIZER;
  this->host_lock = PTHREAD_MUTEX_INITIALIZER;
  this->worker_cond = PTHREAD_COND_INITIALIZER;
  this->worker_lock = PTHREAD_MUTEX_INITIALIZER;
  this->unclaimed = 0;

  for (int i = 0; i < TPOOL_WORKERS; i++) {
    pthread_create(&this->workers[i], NULL, worker, (void *)this);
  }
}

__attribute__((constructor))
static void mod_init(void) {
  memset((void *)taskBuf, 0, sizeof(taskBuf));
}

