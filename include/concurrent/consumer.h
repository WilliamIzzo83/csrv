#include "concurrent/job.h"
#include "concurrent/queue.h"
#include <thread>
#include <iostream>

#ifndef WI_CONCURRENT_CONSUMER_H_
#define WI_CONCURRENT_CONSUMER_H_

namespace wi {
namespace concurrent {

static size_t boia = 0;

template<typename T>
class consumer : job {

public:
  ~consumer() {
    end();
  }

  void start(queue<T>* queue) {
    id_ = ++boia;
    start();
    queue_ = queue;
    worker_thread_ = std::thread(jobExecutorFn, (job*)this);  
    worker_thread_.detach(); 
  }

  virtual void start() {
    job::start();
  }

  virtual void end() {
    job::end();
  }

  virtual void main() {
    while(true) {
      T resource = queue_->dequeue();
      resource.main();
    }
  }

private:
  queue<T>* queue_;
  std::thread worker_thread_;
  size_t id_;
};

}; // concurrent
}; // wi

#endif // WI_CONCURRENT_CONSUMER_H_

