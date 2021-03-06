#ifndef ROBOT_H
#define ROBOT_H

#include <cpen333/thread/thread_object.h>
#include <iostream>
#include <thread>
#include <deque>
#include "CircularOrderQueue.h"
#include "safe_printf.h"
#include "SharedData.h"
#include "Constants.h"
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <cpen333/process/semaphore.h>
#include "Inventory.h"

class Robot : public cpen333::thread::thread_object {
  CircularOrderQueue& orders_;
  int id_;
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;
  cpen333::process::semaphore docks_semaphore;
  std::deque<std::pair<int,int>> path;
  // local copy of warehouse
  WarehouseInfo minfo_;
  int end_col = 0;
  int end_row = 0;
  int order_id, quantity, add, num_docks, isdock, order_quantity;
  std::string product;
  Inventory& inv;
  int dock[MAX_WAREHOUSE_DOCKS][2];
  Coordinate home;
  std::tuple<int,int> coordinates;
  // runner info
  size_t idx_;   // runner index
  int loc_[2];   // current location
 public:
  Robot(int id, CircularOrderQueue& orders, Inventory& inv) :
      id_(id), orders_(orders), memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME),
      minfo_(), idx_(0), loc_(), docks_semaphore(DOCKS_SEMAPHORE_NAME), inv(inv)
  {
    char def_prod[] = "N/A";
    // copy warehouse contents
    minfo_ = memory_->minfo;

    {
      // protect access of number of runners
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      idx_ = memory_->rinfo.nrobot;
      memory_->rinfo.nrobot++;
      home.col = memory_->rinfo.rloc[idx_][COL_IDX];
      home.row = memory_->rinfo.rloc[idx_][ROW_IDX];
    }

    // get current location
    loc_[COL_IDX] = memory_->rinfo.rloc[idx_][COL_IDX];
    loc_[ROW_IDX] = memory_->rinfo.rloc[idx_][ROW_IDX];
    memory_->rinfo.busy[idx_] = 0;
    memory_->rinfo.task[idx_] = 0;
    memory_->rinfo.quantity[idx_] = 0;
    memory_->rinfo.home[idx_] = 1;
    memory_->rinfo.dock[idx_] = 0;

    for(int i=0;i<4;i++) {
      memory_->rinfo.product[idx_][i] = def_prod[i];
    }
    num_docks = memory_->minfo.num_docks;
    for(int i=0;i<num_docks;i++){
      dock[i][COL_IDX] = memory_->minfo.dock_col[i];
      dock[i][ROW_IDX] = memory_->minfo.dock_row[i];
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
    int choice[4];
    int dir;
    int dcol,drow;
    if (memory_->quit) return -1;
    // Make the move. If it's wrong, we will backtrack later.
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

    dcol = col - end_col;
    drow = row - end_row;

    if(dcol != 0){
      if(dcol > 0) {
        choice[0] = 4;
        choice[1] = 2;
      }
      else{
        choice[0] = 2;
        choice[1] = 4;
      }
      if(drow > 0){
        choice[2] = 1;
        choice[3] = 3;
      }
      else{
        choice[2] = 3;
        choice[3] = 1;
      }
    }
    else{
      if(drow > 0){
        choice[0] = 1;
        choice[1] = 3;
      }
      else{
        choice[0] = 3;
        choice[1] = 1;
      }
      if(dcol > 0){
        choice[2] = 4;
        choice[3] = 2;
      }
      else{
        choice[2] = 2;
        choice[3] = 4;
      }
    }

    for(int i=0;i<4;i++) {
      dir = choice[i];

      if(dir == 4) {
        if (col > 0 && (minfo_.warehouse[col - 1][row] != WALL_CHAR && minfo_.warehouse[col - 1][row] != SHELF_CHAR)
            && minfo_.warehouse[col - 1][row] != DOCK_CHAR && minfo_.visited[col - 1][row] == 0 && this->find_path(col - 1, row)) {
          return true;
        }
      }
      else if(dir == 2) {
        if (col < minfo_.cols && (minfo_.warehouse[col + 1][row] != WALL_CHAR && minfo_.warehouse[col + 1][row] != SHELF_CHAR)
            && minfo_.warehouse[col + 1][row] != DOCK_CHAR && minfo_.visited[col + 1][row] == 0 && this->find_path(col + 1, row)) {
          return true;
        }
      }
      else if(dir == 1) {
        if (row > 0 && (minfo_.warehouse[col][row - 1] != WALL_CHAR && minfo_.warehouse[col][row - 1] != SHELF_CHAR)
            && minfo_.warehouse[col][row - 1] != DOCK_CHAR && minfo_.visited[col][row - 1] == 0 && this->find_path(col, row - 1)) {
          return true;
        }
      }
      else if(dir == 3) {
        if (row < minfo_.rows && (minfo_.warehouse[col][row + 1] != WALL_CHAR && minfo_.warehouse[col][row + 1] != SHELF_CHAR)
            && minfo_.warehouse[col][row + 1] != DOCK_CHAR && minfo_.visited[col][row + 1] == 0 && this->find_path(col, row + 1)) {
          return true;
        }
      }
    }

    // Otherwise we need to backtrack and find another solution.
    minfo_.visited[col][row] = 0;
    path.pop_back();

    return false;
  }

  void go(){
    bool blocked = true;
    for(auto& coordinate : path) {
      std::this_thread::sleep_for(std::chrono::milliseconds(70));
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

  int get_available_dock(){
    int curr_dock;
    docks_semaphore.wait();
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      curr_dock = memory_->minfo.curr_dock;
      if (curr_dock < memory_->minfo.num_docks) {
        end_col = memory_->minfo.dock_col[curr_dock];
        end_row = memory_->minfo.dock_row[curr_dock];
        memory_->minfo.curr_dock++;
      }
    }
    return curr_dock + 1;
  }

  void init_order(std::vector<Coordinate>& orders){
    quantity = orders[0].quantity;
    order_id = orders[0].order_id;
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->rinfo.busy[idx_] = 1;
      memory_->rinfo.quantity[idx_] = quantity;
      add = orders[0].add;
      memory_->rinfo.task[idx_] = add;
      if(add) order_quantity = orders[1].quantity;
      else order_quantity = orders[0].quantity;
      product = orders[0].product;
      for(int i=0;i<orders[0].product.length();i++){
        memory_->rinfo.product[idx_][i] = orders[0].product[i];
      }
      memory_->rinfo.product[idx_][orders[0].product.length()] = '\0';
      memory_->rinfo.home[idx_] = 0;
      memory_->rinfo.dock[idx_] = 0;
    }
    orders.push_back({home.row,home.col});
  }

  void order_finish(){
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->rinfo.busy[idx_] = 0;
      memory_->rinfo.home[idx_] = 1;
      memory_->minfo.order_status[add][order_id]--;
      memory_->rinfo.dest[idx_][COL_IDX] = 0;
      memory_->rinfo.dest[idx_][ROW_IDX] = 0;
      inv.update_inv(product, order_quantity, add);
    }
  }

  int main() {
    char cmd = 0;
    if (!(this->check_magic())) {
      safe_printf("Shared memory not initialized\n");
      return 0;
    }
    int x = this->get_start_col();
    int y = this->get_start_row();
    std::vector<Coordinate> orders;
    while (true) {
      orders = orders_.get();
      if (orders[0].row == 999 && orders[0].col == 999) {
        memory_->rinfo.nrobot--;
        break;
      }
      this->init_order(orders);
      for (auto &order:orders) {
        end_col = order.col;
        end_row = order.row;
        if(order.col == -1 && order.row == -1) isdock = get_available_dock();
        if (this->find_path(x, y)) {
          std::tie(y, x) = coordinates;
          this->go();
          {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            memory_->rinfo.dest[idx_][COL_IDX] = x;
            memory_->rinfo.dest[idx_][ROW_IDX] = y;
            memory_->rinfo.quantity[idx_] = order.quantity;
            if(x == home.col && y == home.row)
              this->order_finish();
            if(isdock) memory_->rinfo.dock[idx_] = isdock;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1500));

          if(isdock) {
            {
              std::lock_guard<decltype(mutex_)> lock(mutex_);
              memory_->minfo.curr_dock--;
            }
            isdock = 0;
            docks_semaphore.notify();
          }

        } else {
          safe_printf("Failed to find destination\n");
        }
      }

    }
    safe_printf("Robot %d done\n", id_);

    return 0;
  }
};
#endif //ROBOT_H
