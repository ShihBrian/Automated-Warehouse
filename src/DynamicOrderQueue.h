#ifndef LAB6_DYNAMICORDERQUEUE_H
#define LAB6_DYNAMICORDERQUEUE_H

#include "OrderQueue.h"
#include <deque>
#include <condition_variable>
#include <mutex>

/**
 * Dynamically-sized Queue Implementation
 *
 * Does not block when adding items
 */
class DynamicOrderQueue : public virtual OrderQueue {
  std::deque<Order> buff_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool ready = false;
 public:
  /**
   * Creates the dynamic queue
   */
  DynamicOrderQueue() :
      buff_(), mutex_(), cv_(){}

  void add(const Order& order) {

    {
      std::lock_guard<decltype(mutex_)> lock_guard(mutex_);
      buff_.push_back(order);
      ready = true;
    }
    cv_.notify_one();
  }

  Order get() {
    {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      while(!ready) {
        cv_.wait(lock);
      }
    }
    // get first item in queue
    Order out = buff_.front();
    buff_.pop_front();
    if(buff_.size() == 0){
      {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        ready = false;
      }
    }
    return out;
  }
};

#endif //LAB6_DYNAMICORDERQUEUE_H
