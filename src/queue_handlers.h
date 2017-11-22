#ifndef THREAD_OBJECTS_H
#define THREAD_OBJECTS_H

#include "server.h"
#include "Constants.h"

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
              std::cout << "Delivery or restock is done" << std::endl;
              memory_->minfo.order_status[add][j] = -1;
              memory_->minfo.done[add] = 1;
            }
          }
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
  }

};
#endif //THREAD_OBJECTS_H
