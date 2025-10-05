#include "thread.h"

#include <string.h>

void Task::waitfor(void) {
  pthread_mutex_lock(&this->lock);
  while (this->finished == 0) {
    pthread_cond_wait(&this->host_cond, &this->lock);
  }
  this->finished = 0;
  pthread_mutex_unlock(&this->lock);
}

static void *worker(void *pool) {
  ThreadPool *tpool = (ThreadPool *)pool;

  for (;;) {
    Task *task = tpool->Execute();
    if (!task->routine) {
      /* Be asked to exit. */
      break;
    }

    /* Actually execute. */
    task->routine(task->args);

    /* Notify host thread. */
    pthread_mutex_lock(&task->lock);
    task->finished = 1;
    pthread_mutex_unlock(&task->lock);
    pthread_cond_signal(&task->host_cond);
  }

  pthread_exit((void *) 0);
}

Task *ThreadPool::Execute(void) {
  Task *ret;
  pthread_mutex_lock(&this->lock);
  while (this->nextSlot == 0) {
    pthread_cond_wait(&this->cond, &this->lock);
  }
  this->nextSlot --;
  ret = this->tasks[this->nextSlot];
  pthread_mutex_unlock(&this->lock);

  return ret;
}

void ThreadPool::AddTasks(Task *buf, int count) {
  pthread_mutex_lock(&this->lock);
  for (int i = 0; i < count; i++) {
    this->tasks[this->nextSlot] = &buf[i];
    this->nextSlot ++;
  }
  pthread_mutex_unlock(&this->lock);
  for (int i = 0; i < count; i++) {
    pthread_cond_signal(&this->cond);
  }
}


ThreadPool::~ThreadPool() {
  Task exit_tasks[TPOOL_WORKERS];
  memset((void *)exit_tasks, 0, sizeof(exit_tasks));  
  this->AddTasks(exit_tasks, TPOOL_WORKERS);

  for (int i = 0; i < TPOOL_WORKERS; i++) {
    pthread_join(this->workers[i], NULL);
  }
}

ThreadPool::ThreadPool() {
  this->cond = PTHREAD_COND_INITIALIZER;
  this->lock = PTHREAD_MUTEX_INITIALIZER;
  this->nextSlot = 0;
  for (int i = 0; i < TPOOL_WORKERS; i++) {
    this->tasks[i] = NULL;
  }
  for (int i = 0; i < TPOOL_WORKERS; i++) {
    pthread_create(&this->workers[i], NULL, worker, (void *)this);
  }
}
