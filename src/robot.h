#ifndef ROBOT_H
#define ROBOT_H

#include <cpen333/thread/thread_object.h>
#include <iostream>
#include <thread>
#include <deque>
#include "OrderQueue.h"
#include "safe_printf.h"
#include "warehouse_layout.h"
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <cpen333/process/semaphore.h>

class Robot : public cpen333::thread::thread_object {
  OrderQueue& orders_;
  int id_;
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;
  cpen333::process::semaphore docks_semaphore;
  std::deque<std::pair<int,int>> path;
  // local copy of warehouse
  WarehouseInfo minfo_;
  int end_col = 0;
  int end_row = 0;
  std::string product;
  int quantity;
  Coordinate home;
  std::tuple<int,int> coordinates;
  // runner info
  size_t idx_;   // runner index
  int loc_[2];   // current location
 public:
  Robot(int id, OrderQueue& orders) :
      id_(id), orders_(orders), memory_(MAZE_MEMORY_NAME), mutex_(MAZE_MUTEX_NAME),
      minfo_(), idx_(0), loc_(), docks_semaphore(DOCKS_SEMAPHORE_NAME)
  {
    char def_prod[] = "N/A";
    // copy warehouse contents
    minfo_ = memory_->minfo;

    {
      // protect access of number of runners
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      idx_ = memory_->rinfo.nrobot;
      memory_->rinfo.nrobot++;
      home.col = memory_->minfo.home_col;
      home.row = memory_->minfo.home_row;
    }

    // get current location
    loc_[COL_IDX] = memory_->rinfo.rloc[idx_][COL_IDX];
    loc_[ROW_IDX] = memory_->rinfo.rloc[idx_][ROW_IDX];
    memory_->rinfo.busy[idx_] = 0;
    memory_->rinfo.task[idx_] = 0;
    memory_->rinfo.quantity[idx_] = 0;
    for(int i=0;i<4;i++) {
      memory_->rinfo.product[idx_][i] = def_prod[i];
    }
  }

  int get_start_row() {
    return loc_[ROW_IDX];
  }

  int get_start_col() {
    return loc_[COL_IDX];
  }

  int check_magic() {
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      if (memory_->magic == MAGIC)
        return 1;
      else return 0;
    }
  }

  int find_path(int col, int row) {
    if (memory_->quit) return -1;
    // Make the move (if it's wrong, we will backtrack later.
    minfo_.visited[col][row] = 1;
    path.push_back({col,row});
    // Check if we have reached our goal.
    if (col == end_col && row == end_row) {
      coordinates = std::make_tuple(row,col);
      //clear visited array
      for (int i=0; i<minfo_.rows;i++){
        for(int j=0;j<minfo_.cols;j++){
          minfo_.visited[j][i] = 0;
        }
      }
      return true;
    }
    // Recursively search for our goal.
    if (col > 0 && (minfo_.warehouse[col - 1][row] != WALL_CHAR && minfo_.warehouse[col - 1][row] != SHELF_CHAR) && minfo_.visited[col - 1][row] == 0 && this->find_path(col - 1, row)) {
      return true;
    }
    if (col < minfo_.cols && (minfo_.warehouse[col + 1][row] != WALL_CHAR && minfo_.warehouse[col + 1][row] != SHELF_CHAR) && minfo_.visited[col + 1][row] == 0 && this->find_path(col + 1, row)) {
      return true;
    }
    if (row > 0 && (minfo_.warehouse[col][row - 1] != WALL_CHAR && minfo_.warehouse[col][row - 1] != SHELF_CHAR) && minfo_.visited[col][row - 1] == 0 && this->find_path(col, row - 1)) {
      return true;
    }
    if (row < minfo_.rows && (minfo_.warehouse[col][row + 1] != WALL_CHAR && minfo_.warehouse[col][row + 1] != SHELF_CHAR) && minfo_.visited[col][row + 1] == 0 && this->find_path(col, row + 1)) {
      return true;
    }

    // Otherwise we need to backtrack and find another solution.
    minfo_.visited[col][row] = 0;
    path.pop_back();

    return false;
  }

  void go(){
    for(auto& coordinate : path) {
      std::this_thread::sleep_for(std::chrono::milliseconds(60));
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        memory_->rinfo.rloc[idx_][COL_IDX] = coordinate.first;
        memory_->rinfo.rloc[idx_][ROW_IDX] = coordinate.second;
      }
    }
    path.clear();
  }

  int id() {
    return id_;
  }

  void init_order(std::vector<Coordinate>& orders){
    Coordinate dock;
    int curr_dock;
    docks_semaphore.wait();
    quantity = orders[0].quantity;
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      curr_dock = memory_->minfo.curr_dock;
      if(curr_dock < memory_->minfo.num_docks) {
        dock.col = memory_->minfo.dock_col[curr_dock];
        dock.row = memory_->minfo.dock_row[curr_dock];
        memory_->minfo.curr_dock++;
        memory_->rinfo.busy[idx_] = 1;
        memory_->rinfo.quantity[idx_] = quantity;
        memory_->rinfo.task[idx_] = orders[0].add;
        for(int i=0;i<orders[0].product.length();i++){
          memory_->rinfo.product[idx_][i] = orders[0].product[i];
        }
        memory_->rinfo.product[idx_][orders[0].product.length()+1] = '\0';
      }
    }

    for(auto& order : orders){
      if(order.col == -1 && order.row == -1){
        order.col = dock.col;
        order.row = dock.row;
      }
    }
    orders.push_back({home.row,home.col});
  }

  void order_finish(){
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->minfo.curr_dock--;
      memory_->rinfo.busy[idx_] = 0;
    }
    docks_semaphore.notify();
  }

  //TODO: Get stock from truck or move stock to delivery truck depending on order
  //TODO: free dock immediately after visiting
  int main() {
    bool quit = false;
    char cmd = 0;
    if (!(this->check_magic())) {
      safe_printf("Shared memory not initialized\n");
      return 0;
    }
    int x = this->get_start_col();
    int y = this->get_start_row();
    std::vector<Coordinate> orders;
    while (!quit) {
      orders = orders_.get();
      this->init_order(orders);
      for (auto &order:orders) {
        if (order.row == 999 && order.col == 999) {
          quit = true;
          break;
        }
        end_col = order.col;
        end_row = order.row;
        if (this->find_path(x, y)) {
          std::tie(y, x) = coordinates;
          this->go();
          memory_->rinfo.dest[idx_][COL_IDX] = x;
          memory_->rinfo.dest[idx_][ROW_IDX] = y;
          memory_->rinfo.quantity[idx_] = order.quantity;
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
          safe_printf("Failed to find destination\n");
        }
      }
      this->order_finish();
    }
    safe_printf("Robot %d done\n", id_);

    return 0;
  }
};
#endif //ROBOT_H
