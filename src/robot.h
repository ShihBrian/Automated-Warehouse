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

/**
 * The Robot grabs orders from a queue, cooks them,
 * then adds the cooked dishes to a new queue for
 * the servers to serve
 */
class Robot : public cpen333::thread::thread_object {
  OrderQueue& orders_;
  OrderQueue& serve_;
  int id_;
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;
  std::deque<std::pair<int,int>> path;
  // local copy of maze
  MazeInfo minfo_;
  int end_col = 0;
  int end_row = 0;
  std::tuple<int,int> coordinates;
  // runner info
  size_t idx_;   // runner index
  int loc_[2];   // current location
 public:
  /**
   * Create a new chef
   * @param id the chef's id
   * @param orders queue to read orders from
   * @param serve queue to add completed orders to
   */
  Robot(int id, OrderQueue& orders, OrderQueue& serve) :
      id_(id), orders_(orders), serve_(serve),memory_(MAZE_MEMORY_NAME), mutex_(MAZE_MUTEX_NAME),
      minfo_(), idx_(0), loc_()
  {
    // copy maze contents
    minfo_ = memory_->minfo;

    {
      // protect access of number of runners
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      idx_ = memory_->rinfo.nrunners;
      memory_->rinfo.nrunners++;
    }

    // get current location
    loc_[COL_IDX] = memory_->rinfo.rloc[idx_][COL_IDX];
    loc_[ROW_IDX] = memory_->rinfo.rloc[idx_][ROW_IDX];
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
    if (col > 0 && (minfo_.maze[col - 1][row] != WALL_CHAR && minfo_.maze[col - 1][row] != SHELF_CHAR) && minfo_.visited[col - 1][row] == 0 && this->find_path(col - 1, row)) {
      return true;
    }
    if (col < minfo_.cols && (minfo_.maze[col + 1][row] != WALL_CHAR && minfo_.maze[col + 1][row] != SHELF_CHAR) && minfo_.visited[col + 1][row] == 0 && this->find_path(col + 1, row)) {
      return true;
    }
    if (row > 0 && (minfo_.maze[col][row - 1] != WALL_CHAR && minfo_.maze[col][row - 1] != SHELF_CHAR) && minfo_.visited[col][row - 1] == 0 && this->find_path(col, row - 1)) {
      return true;
    }
    if (row < minfo_.rows && (minfo_.maze[col][row + 1] != WALL_CHAR && minfo_.maze[col][row + 1] != SHELF_CHAR) && minfo_.visited[col][row + 1] == 0 && this->find_path(col, row + 1)) {
      return true;
    }

    // Otherwise we need to backtrack and find another solution.
    minfo_.visited[col][row] = 0;
    path.pop_back();

    return false;
  }

  void go(){
    for(auto& coordinate : path) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
  /**
   * Main execution function
   * @return 0 if completed
   */
  int main() {
    bool quit = false;
    char cmd = 0;
    int home_col = 1;
    int home_row = 18;
    if (!(this->check_magic())) {
      safe_printf("Shared memory not initialized\n");
      return 0;
    }
    int x = this->get_start_col();
    int y = this->get_start_row();
    int i = 0;
    std::vector<Order> orders = orders_.get();
    while (!quit) {
      int j = 0;
      for(auto& order:orders){
        safe_printf("Robot %d Path %d Row: %d Col: %d\n",id_,j,order.row,order.col);
        j++;
      }
      for (auto &order:orders) {
        if (order.row == 999 && order.col == 999) quit = true;
        end_col = order.col;
        end_row = order.row;
        safe_printf("Robot %d finding path %d\n", id_, i);
        if (this->find_path(x, y)) {
          std::tie(y, x) = coordinates;
          this->go();
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
          safe_printf("Failed to find destination\n");
        }
        i++;
        safe_printf("Robot %d finished path %d\n", id_, i);
      }

      orders = orders_.get();
    }

    safe_printf("Robot %d done\n", id_);

    return 0;
  }
};
#endif //ROBOT_H
