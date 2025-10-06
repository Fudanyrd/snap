#include "thread.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define N (TPOOL_WORKERS * 1024)

static int A[N];
static int B[N];

static void init_task(void **args) {
  /* int *arr, int value, int len */

  int *arr = (int *)args[0];
  int value = (intptr_t)args[1];
  int len = (intptr_t) args[2];

  for (int i = 0; i < len; i++) {
    arr[i] = value;
  }
}

static void prefixsum_task(void **args) {
  /* int *in, int *out, int len */

  int *in = (int *)args[0];
  int *out = (int *)args[1];
  int len = (intptr_t) args[2];

  out[0] = in[0];
  for (int i = 1; i < len; i++) {
    out[i] = out[i - 1] + in[i];
  }
}

static void add_task(void **args) {
  /* int *arr, int value, int len */

  int *arr = (int *)args[0];
  int value = (intptr_t)args[1];
  int len = (intptr_t)args[2];

  for (int i = 0; i < len; i++) {
    arr[i] += value;
  }
}

int main(int argc, char **argv) {
  ThreadPool pool;
  /* Initialize buffer */
  const int seglen = N / TPOOL_WORKERS;
  for (int i = 0; i < TPOOL_WORKERS; i++) {
    Task &t = taskBuf[i];
    t.routine = init_task;
    t.args[0] = (void *)&(A[i * seglen]);
    t.args[1] = (void *) 1;
    t.args[2] = (void *)seglen;
    pool.push(&t);
  }
  pool.waitForAll();
  for (int i = 0; i < N; i++) {
    if (A[i] != 1) {
      fprintf(stderr, "At i = %d\n", i);
      abort();
    }
  }
  
  for (int i = 0; i < TPOOL_WORKERS; i++) {
    Task &t = taskBuf[i];
    t.routine = prefixsum_task;
    t.args[0] = (void *)&(A[i * seglen]);
    t.args[1] = (void *)&(B[i * seglen]);
    t.args[2] = (void *)seglen;
    pool.push(&t);
  }
  pool.waitForAll();

  int aux[TPOOL_WORKERS];
  aux[0] = B[seglen - 1];
  for (int i = 1; i < TPOOL_WORKERS; i++) {
    Task &t = taskBuf[i];
    const int last = aux[i - 1];
    aux[i] = B[(i + 1) * seglen - 1] + last;
    t.routine = add_task;
    t.args[0] = (void *)&(B[i * seglen]);
    t.args[1] = (void *) last;
    t.args[2] = (void *)seglen;
    pool.push(&t);
  }
  pool.waitForAll();

  for (int i = 0; i < N; i++) {
    // assert(B[i] == i + 1);
    if (B[i] != i + 1) {
      fprintf(stderr, "At i = %d, B[i] = %d\n", i, B[i]);
      abort();
    }
  }

  return 0;
}
