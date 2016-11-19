#ifndef WI_CONCURRENT_QUEUE_H_
#define WI_CONCURRENT_QUEUE_H_

#include <list>
#include <mutex>
#include <condition_variable>

namespace wi {
namespace concurrent {

template<typename T>
class queue {
public:
  void enqueue(T item) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push_back(item);
    queue_wait_.notify_one();
  }

  T dequeue() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if ( queue_.empty() ) {
      queue_wait_.wait(lock, [this]() { return !queue_.empty(); });
    }
    
    T item = queue_.front();
    queue_.pop_front();
    return item;  
  }
 
private:
  std::list<T> queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_wait_;
};

}; // concurrent
}; // namespace

#endif // WI_CONCURRENT_QUEUE_H_

