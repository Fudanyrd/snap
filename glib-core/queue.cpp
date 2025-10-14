#include "queue.h"
#include <utility>

template struct ConcurrentQueueNode<int>;
template class ConcurrentQueue<int>;

template class ConcurrentQueue<std::pair<int, int> >;
template struct ConcurrentQueueNode<std::pair<int, int> >;
