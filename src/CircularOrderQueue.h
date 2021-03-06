#ifndef CIRCULARORDERQUEUE_H
#define CIRCULARORDERQUEUE_H

#include <cpen333/thread/semaphore.h>
#include <mutex>

#define CIRCULAR_BUFF_SIZE 20

struct Coordinate {
  int row;
  int col;
  std::string product;
  int quantity;
  int add;
  int order_id;
};

/**
 * Queue implementation using a circular buffer
 * (i.e. a fixed-size queue)
 */
class CircularOrderQueue{
  std::vector<Coordinate> buff_[CIRCULAR_BUFF_SIZE];
  cpen333::thread::semaphore producer_;
  cpen333::thread::semaphore consumer_;
  std::mutex pmutex_;
  std::mutex cmutex_;
  size_t pidx_;
  size_t cidx_;


 public:
  /**
   * Creates a queue with provided circular buffer size
   * @param buffsize size of circular buffer
   */
  CircularOrderQueue() :
      buff_(),
      producer_(CIRCULAR_BUFF_SIZE), consumer_(0),
      pmutex_(), cmutex_(), pidx_(0), cidx_(0){}

  void add(std::vector<Coordinate>& order) {
    producer_.wait();
    int pidx;
    pmutex_.lock();
    pidx = pidx_;
    // update producer index
    pidx_ = (pidx_+1)%CIRCULAR_BUFF_SIZE;
    pmutex_.unlock();
    buff_[pidx] = order;
    consumer_.notify();
  }

  std::vector<Coordinate> get() {
    consumer_.wait();
    int cidx;
    cmutex_.lock();
    cidx = cidx_;
    // update consumer index
    cidx_ = (cidx_+1)%CIRCULAR_BUFF_SIZE;
    cmutex_.unlock();
    std::vector<Coordinate> out = buff_[cidx];
    producer_.notify();
    return out;
  }

};

#endif //CIRCULARORDERQUEUE_H
