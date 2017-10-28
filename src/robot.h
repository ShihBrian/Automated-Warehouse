#ifndef ROBOT_H
#define ROBOT_H

#include <cpen333/thread/thread_object.h>
#include <iostream>
#include <thread>

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
  int go(int col, int row) {
    if (memory_->quit) return -1;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->rinfo.rloc[idx_][COL_IDX] = col;
      memory_->rinfo.rloc[idx_][ROW_IDX] = row;
    }
    // Make the move (if it's wrong, we will backtrack later.
    minfo_.visited[col][row] = 1;

    // Check if we have reached our goal.
    if (col == end_col && row == end_row) {
      coordinates = std::make_tuple(row,col);
      for (int i=0; i<minfo_.rows;i++){
        for(int j=0;j<minfo_.cols;j++){
          minfo_.visited[j][i] = 0;
        }
      }
      return true;
    }
    // Recursively search for our goal.
    if (col > 0 && (minfo_.maze[col - 1][row] != WALL_CHAR && minfo_.maze[col - 1][row] != SHELF_CHAR) && minfo_.visited[col - 1][row] == 0 && this->go(col - 1, row)) {
      return true;
    }
    if (col < minfo_.cols && (minfo_.maze[col + 1][row] != WALL_CHAR && minfo_.maze[col + 1][row] != SHELF_CHAR) && minfo_.visited[col + 1][row] == 0 && this->go(col + 1, row)) {
      return true;
    }
    if (row > 0 && (minfo_.maze[col][row - 1] != WALL_CHAR && minfo_.maze[col][row - 1] != SHELF_CHAR) && minfo_.visited[col][row - 1] == 0 && this->go(col, row - 1)) {
      return true;
    }
    if (row < minfo_.rows && (minfo_.maze[col][row + 1] != WALL_CHAR && minfo_.maze[col][row + 1] != SHELF_CHAR) && minfo_.visited[col][row + 1] == 0 && this->go(col, row + 1)) {
      return true;
    }

    // Otherwise we need to backtrack and find another solution.
    minfo_.visited[col][row] = 0;
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->rinfo.rloc[idx_][COL_IDX] = col;
      memory_->rinfo.rloc[idx_][ROW_IDX] = row;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return false;
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

    Order order = orders_.get();
    while (true) {
      if (order.row == 999 && order.col == 999) break;
      end_col = order.col;
      end_row = order.row;
      if(this->go(x,y)){
        std::tie(y,x) = coordinates;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        end_col = home_col;
        end_row = home_row;
        if(this->go(x,y)) std::tie(y,x) = coordinates;
      }
      else{
        safe_printf("Failed to find destination\n");
      }
      // next order
      order = orders_.get();
    }

    safe_printf("Robot %d done\n", id_);

    return 0;
  }
};
#endif //ROBOT_H
