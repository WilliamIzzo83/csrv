#include "concurrent/queue.h"
#include "concurrent/consumer.h"
#include <array>

#ifndef WI_CONCURRENT_PRODUCER_H_
#define WI_CONCURRENT_PRODUCER_H_

namespace wi {
namespace concurrent {

template<typename T>
class producer {
public:
  producer() {
    for(int widx = 0; widx < workers_count_; ++widx) {
      consumers_[widx].start(&queue_);
    }
  } 

  void dispatch(T resource) {
     queue_.enqueue(resource);
  }

private:
  queue<T> queue_;
  static const int workers_count_ = 4;
  std::array<consumer<T>,workers_count_> consumers_;
};

}; // concurrent
}; // wi

#endif // WI_CONCURRENT_PRODUCER_H_
