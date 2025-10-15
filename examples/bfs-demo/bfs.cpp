/**
 * A concurrent Breadth-first search implementation.
 * Version 1.0 and before is based on a concurrent queue(with lock).
 * From version 2.0, it uses a lock-free ring buffer instead. 
 * 
 * #define __BFS_VERSION_MAJOR__
 */
#include "bitset.h"
#include "ringbuf.h"
#include "queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#define __BFS_VERSION_MAJOR__ 2

struct ScopeLock {
  pthread_spinlock_t *lock;

  ScopeLock(pthread_spinlock_t *l) {
    lock = l;
    pthread_spin_lock(lock);
  }

  ~ScopeLock() {
    pthread_spin_unlock(lock);
  }
};

using std::vector;

#if  __BFS_VERSION_MAJOR__ <= 1 
static void bfsTask(void **args) {
  const vector<vector<int> > &edges = *(const vector<vector<int> > *) args[0];
  BitSet &bitset = *(BitSet *) args[1];
  vector<ConcurrentQueueNode<int> > &nodes = *(vector<ConcurrentQueueNode<int> > *)args[2];
  int *depths = (int *)args[3];
  ConcurrentQueue<int> &srcQ = *(ConcurrentQueue<int> *) args[4];
  ConcurrentQueue<int> &dstQ = *(ConcurrentQueue<int> *) args[5];

  ConcurrentQueueNode<int> *node;
  while ((node = srcQ.pop()) != 0) {
    const int start = node->item;
    const int depth = depths[start];
    const int newDepth = depth + 1;

    size_t len = edges[start].size();
    for (size_t i = 0; i < len; i++) {
      int succ = edges[start][i];
      if (bitset.testAndSet(succ))
        continue;
      
      
      /* Because of `bitset`, the node at `succ` can be safely written,
       * i.e. it is impossible for two threads to use `nodes[succ]` simultaneously.
       */
      ConcurrentQueueNode<int> *nextNode = (nodes.data()) + succ;
      nextNode->item = succ;

      /*
       * We can safely set `depths[succ]`. If `depths[succ]` is set to
       * some values earlier, the bitset will not permit us to reach here.
       */
      depths[succ] = newDepth;
      dstQ.push(nextNode);
    }
  }
}

void bfs(vector<vector<int> > &edges, int start, vector<int> &depths) {
  const int n = edges.size();
  /* Allocate space for queue nodes. */
  std::vector<ConcurrentQueueNode<int> > nodes(n);

  /* Allocate a dst queue for each worker. */
  ConcurrentQueue<int> dstQueues[TPOOL_WORKERS];
  ConcurrentQueue<int> srcQueue, tmpQueue;

  BitSet bitset(n);
  bitset.testAndSet(start);
  depths[start] = 0;

  /* Deal with successor of `start` in single thread. */
  size_t len = edges[start].size();
  for (size_t i = 0; i < len; i++) {
    int succ = edges[start][i];
    depths[succ] = 1;
    bitset.testAndSet(succ);
    ConcurrentQueueNode<int> *node = nodes.data() + succ;
    node->item = succ;
    srcQueue.push(node);
  }

  for (;;) {
    for (int i = 0; i < TPOOL_WORKERS; i++) {
      Task *t = &taskBuf[i];
      t->args[0] = &edges;
      t->args[1] = &bitset;
      t->args[2] = &nodes;
      t->args[3] = depths.data();
      t->args[4] = &srcQueue;
      t->args[5] = &dstQueues[i];
      t->routine = bfsTask;

      tpool.push(t);
    }

    /*
     * for (int i = 0; i < TPOOL_WORKERS; i++)
     *   pthread_join(&worker[i]);
     */
    bool cont = false;
    Task *fini;
    int collected = 0;
    while ((fini = tpool.waitFor())  != 0) {
      /* Recover the tasks to do for the next iteration. */
      cont |= tmpQueue.append(*(ConcurrentQueue<int> *) fini->args[5]);
      collected ++;
    }

    /* Should collect all workers, else there's bug in thread pool. */
    assert (collected == TPOOL_WORKERS);

    if (!cont) { break; }

    /* The queue is not empty. should continue. */
    srcQueue.append(tmpQueue);
  }
}

#else

static void bfsTask(void **args) {
  const vector<vector<int> > &edges = *(const vector<vector<int> > *) args[0];
  BitSet &bitset = *(BitSet *) args[1];
  int *depths = (int *)args[2];
  RingBuf<int> &srcQ = *(RingBuf<int> *) args[3];
  RingBuf<int> &dstQ = *(RingBuf<int> *) args[4];

  int *node;
  while ((node = srcQ.pop()) != 0) {
    const int start = *node;
    const int depth = depths[start];
    const int newDepth = depth + 1;

    size_t len = edges[start].size();
    for (size_t i = 0; i < len; i++) {
      int succ = edges[start][i];
      if (bitset.testAndSet(succ))
        continue;
      
      
      /* Because of `bitset`, the node at `succ` can be safely written,
       * i.e. it is impossible for two threads to use `nodes[succ]` simultaneously.
       */
      dstQ.push(succ);

      /*
       * We can safely set `depths[succ]`. If `depths[succ]` is set to
       * some values earlier, the bitset will not permit us to reach here.
       */
      depths[succ] = newDepth;
    }
  }
}

void bfs(vector<vector<int> > &edges, int start, vector<int> &depths) {
  const int n = edges.size();
  RingBuf<int> srcQueue(n), dstQueue(n);

  BitSet bitset(n);
  bitset.testAndSet(start);
  depths[start] = 0;

  /* Deal with successor of `start` in single thread. */
  size_t len = edges[start].size();
  for (size_t i = 0; i < len; i++) {
    int succ = edges[start][i];
    depths[succ] = 1;
    bitset.testAndSet(succ);
    srcQueue.push(succ);
  }

  for (;;) {
    for (int i = 0; i < TPOOL_WORKERS; i++) {
      Task *t = &taskBuf[i];
      t->args[0] = &edges;
      t->args[1] = &bitset;
      t->args[2] = depths.data();
      t->args[3] = &srcQueue;
      t->args[4] = &dstQueue;
      t->routine = bfsTask;

      tpool.push(t);
    }

    /*
     * for (int i = 0; i < TPOOL_WORKERS; i++)
     *   pthread_join(&worker[i]);
     */
    bool cont = false;
    Task *fini;
    int collected = 0;
    while ((fini = tpool.waitFor())  != 0) {
      collected ++;
    }

    /* Should collect all workers, else there's bug in thread pool. */
    assert (collected == TPOOL_WORKERS);

    cont = ! dstQueue.empty();
    if (!cont) { break; }

    /* The queue is not empty. should continue. */
    /* Recover the tasks to do for the next iteration. */
    RingBuf<int>::exchange(srcQueue, dstQueue);
    dstQueue.reset();
  }
}
#endif

int main(int argc, char **argv) {
  int nodes = 1024 * 8;
  vector<vector<int> > edges(nodes, vector<int>(2, 0));

  /* Construct an undirected graph. */
  for (int i = 1; i < nodes; i++) {
    edges[i][0] = i - 1;
    edges[i][1] = i + 1;
  }
  edges[0].clear();
  edges[0].push_back(1);
  edges[nodes - 1].clear(); 
  edges[nodes - 1].push_back(nodes - 2);

  /* Run BFS on it. */
  std::vector<int> depths(nodes, -1);
  bfs(edges, 0, depths);

  /* Verify result. */
  for (int i = 0; i < nodes; i++) 
    if (depths[i] != i) {
      printf("Excepted %d, got %d\n", i, depths[i]);
      abort();
    }

  printf("SUCCESS\n");
  return 0;
}
