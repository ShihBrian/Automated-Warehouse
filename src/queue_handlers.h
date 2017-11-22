#ifndef THREAD_OBJECTS_H
#define THREAD_OBJECTS_H

#include "server.h"
#include "Constants.h"

class dock_monitor : public cpen333::thread::thread_object {
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;
  cpen333::process::semaphore trucks_semaphore;
  CircularOrderQueue& robot_queue;
  CircularOrderQueue& order_queue;
  Inventory& inv;
public:
  dock_monitor(CircularOrderQueue& order_q, Inventory& inv, CircularOrderQueue& robot_q) :
      memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME), trucks_semaphore(TRUCKS_SEMAPHORE_NAME),
      robot_queue(robot_q), inv(inv), order_queue(order_q){}

  int main() {
    std::vector<Coordinate> coordinates;
    std::vector<Coordinate> temp;
    while (true) {
      if (memory_->quit) break;

      coordinates = order_queue.get();
      trucks_semaphore.wait();
      if(coordinates[0].add) memory_->minfo.restock = 1;
      else memory_->minfo.deliver = 1;
      for(int i=0;i<coordinates.size();i+=2){
        temp.push_back(coordinates[i]);
        temp.push_back(coordinates[i+1]);
        robot_queue.add(temp);
        temp.clear();
      }
    }
  }
};

class order_monitor : public cpen333::thread::thread_object {
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;
  cpen333::process::semaphore trucks_semaphore;
public:
  order_monitor() :
      memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME), trucks_semaphore(TRUCKS_SEMAPHORE_NAME) {}
  int main() {
    while (true) {
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(memory_->quit) break;
        for(int j=0;j<MAX_WAREHOUSE_DOCKS;j++){
          for(int add=0;add<2;add++){
            if(memory_->minfo.order_status[add][j] == 0){
              memory_->minfo.order_status[add][j] = -1;
              memory_->minfo.done[add] = 1;
            }
          }
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      trucks_semaphore.notify();
    }
    return 0;
  }

};
#endif //THREAD_OBJECTS_H
