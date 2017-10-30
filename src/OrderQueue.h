#ifndef QUEUE_H
#define QUEUE_H

struct Order {
  int row;
  int col;
};
/**
 * Thread-safe queue of items
 */
class OrderQueue {
 public:
  /**
   * Adds an order item to the queue
   * @param order item to add
   */
  virtual void add(std::vector<Order>& order) = 0;

  /**
   * Retrieve the next order item in the queue
   * @return next available item
   */
  virtual std::vector<Order> get() = 0;

};

#endif //QUEUE_H
