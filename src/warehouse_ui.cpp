#include "SharedData.h"
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <cpen333/console.h>
#include <thread>

/**
* Handles all drawing/memory synchronization for the
* User Interface process
*/
class WarehouseUI {
  // display offset for better visibility
  static const int XOFF = 2;
  static const int YOFF = 1;
  unsigned line_count = 0;
  cpen333::console display_;
  cpen333::process::shared_object<SharedData> memory_;
  cpen333::process::mutex mutex_;

  // previous positions of runners
  int lastpos_[MAX_ROBOTS][2];
  int home_[2];   // exit location
public:

  WarehouseUI() : display_(), memory_(MAZE_MEMORY_NAME), mutex_(MAZE_MUTEX_NAME) {

    // clear display and hide cursor
    display_.clear_all();
    display_.set_cursor_visible(false);

    // initialize last known runner positions
    for (size_t i = 0; i<MAX_ROBOTS; ++i) {
      lastpos_[i][COL_IDX] = -1;
      lastpos_[i][ROW_IDX] = -1;
    }

    home_[COL_IDX] = memory_->minfo.home_col;
    home_[ROW_IDX] = memory_->minfo.home_row;

  }
  /**
  * Draws the maze itself
  */
  void draw_warehouse() {
    static const char WALL = 219;  // WALL character
    static const char EXIT = 176;  // EXIT character
    static const char HOME = 72;  // EXIT character
    static const char SHELF = 83;
    static const char DOCK = 68;
    WarehouseInfo& minfo = memory_->minfo;
    RobotInfo& rinfo = memory_->rinfo;

    // clear display
    display_.clear_display();
    // draw warehouse
    for (int r = 0; r < minfo.rows; ++r) {
      display_.set_cursor_position(YOFF + r, XOFF);
      for (int c = 0; c < minfo.cols; ++c) {
        char ch = minfo.warehouse[c][r];
        if (ch == WALL_CHAR) {
          std::printf("%c", WALL);
        }
        else if (ch == EXIT_CHAR) {
          std::printf("%c", EXIT);
        }
        else if (ch == HOME_CHAR){
          std::printf("%c", HOME);
        }
        else if(ch == SHELF_CHAR){
          std::printf("%c", SHELF);
        }
        else if(ch == DOCK_CHAR){
          std::printf("%c", DOCK);
        }
        else {
          std::printf("%c", EMPTY_CHAR);
        }
      }
    }
  }

  void draw_objects(){
    static const char HOME = 72;  // EXIT character
    static const char DOCK = 68;
    WarehouseInfo& minfo = memory_->minfo;
    RobotInfo& rinfo = memory_->rinfo;

    // draw warehouse
    for (int r = 0; r < minfo.rows; ++r) {
      for (int c = 0; c < minfo.cols; ++c) {
        display_.set_cursor_position(YOFF + r, XOFF+c);
        char ch = minfo.warehouse[c][r];
        if (ch == HOME_CHAR){
          std::printf("%c", HOME);
        }
        else if(ch == DOCK_CHAR){
          std::printf("%c", DOCK);
        }
      }
    }
  }

  int check_magic() {
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      if (memory_->magic == MAGIC)
        return 1;
      else return 0;
    }
  }

  void clear_log(int lines){
    char empty_line[] = "                                                  ";
    for(int i=0;i<=lines;i++){
      display_.set_cursor_position(YOFF + i, XOFF + memory_->minfo.cols + 2);
      std::printf("%s", empty_line);
    }
    display_.set_cursor_position(YOFF, XOFF + memory_->minfo.cols + 2);
  }

  void set_and_check_log(unsigned &line_count){
    display_.set_cursor_position(YOFF + line_count, XOFF + memory_->minfo.cols + 2);
    line_count++;
    if(line_count > memory_->minfo.rows) {
      line_count = 1;
      this->clear_log(memory_->minfo.rows);
    }
  }

  void draw_robots() {
    RobotInfo& rinfo = memory_->rinfo;
    int newc, newr, busy, task, quantity, home;
    int dest[2];
    int home_coord[2];
    char product[MAX_ROBOTS][MAX_WORD_LENGTH];
    int count = 0;
    int dock_num;
    // draw all runner locations
    for (size_t i = 0; i<rinfo.nrobot; ++i) {
      char me = 'A'+i;

      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        newr = rinfo.rloc[i][ROW_IDX];
        newc = rinfo.rloc[i][COL_IDX];
        busy = rinfo.busy[i];
        task = rinfo.task[i];
        home = rinfo.home[i];
        dock_num = rinfo.dock[i];
        home_coord[COL_IDX] = memory_->minfo.home_col;
        home_coord[ROW_IDX] = memory_->minfo.home_row;
        if(home) {
          set_and_check_log(line_count);
          std::printf("Robot %c home", me);
          rinfo.home[i] = 0;
        }
        if(memory_->minfo.restock){
          set_and_check_log(line_count);
          std::printf("Restocking truck arrived at dock");
          memory_->minfo.restock = 0;
        }
        if(memory_->minfo.deliver){
          set_and_check_log(line_count);
          std::printf("Delivery truck arrived at dock");
          memory_->minfo.deliver = 0;
        }
        if(dock_num){
          set_and_check_log(line_count);
          if(task == 1) std::printf("Robot %c unloading product from truck at dock %d",me, dock_num);
          else std::printf("Robot %c loading product onto truck at dock %d",me, dock_num);
          memory_->rinfo.dock[i] = 0;
        }
        while(rinfo.product[i][count] != '\0'){
          product[i][count] = rinfo.product[i][count];
          count++;
        }
        product[i][count] = '\0';
        count = 0;
        quantity = rinfo.quantity[i];
        dest[COL_IDX] = rinfo.dest[i][COL_IDX];
        dest[ROW_IDX] = rinfo.dest[i][ROW_IDX];

        for(int j=0;j<MAX_WAREHOUSE_DOCKS;j++){
          for(int add=0;add<2;add++){
            if(memory_->minfo.order_status[add][j] == 0){
              memory_->minfo.order_status[add][j] = -1;
              set_and_check_log(line_count);
              if(add) std::printf("Restocking truck empty, leaving now");
              else std::printf("Order fulfilled, delivery truck leaving");
            }
          }
        }
      }
      display_.set_cursor_position(YOFF + lastpos_[i][ROW_IDX], XOFF + lastpos_[i][COL_IDX]);
      std::printf("%c", EMPTY_CHAR);
      if(busy) {
        if (newc != lastpos_[i][COL_IDX]
            || newr != lastpos_[i][ROW_IDX]) {
          // zero out last spot and update known location
          lastpos_[i][COL_IDX] = newc;
          lastpos_[i][ROW_IDX] = newr;
          if((newc == dest[COL_IDX] && newr == dest[ROW_IDX]) &&
              (dest[COL_IDX] != home_coord[COL_IDX] && dest[ROW_IDX] != home_coord[ROW_IDX])){
              if(!dock_num) {
                set_and_check_log(line_count);
                if(task == 1) std::printf("Robot %c stocking shelf with %d %s",me, quantity, product[i]);
                else std::printf("Robot %c unloading %d %s from shelf",me, quantity, product[i]);
              }
          }
        }
      }

      // print runner at new locationrinfo
      display_.set_cursor_position(YOFF + newr, XOFF + newc);
      std::printf("%c", me);

    }
  }

  /**
  * Checks if we are supposed to quit
  * @return true if memory tells us to quit
  */
  bool quit() {
    // check if we need to quit
    return memory_->quit;
  }

  ~WarehouseUI() {
    // reset console settings
    display_.clear_all();
    display_.reset();
  }
};

int main() {

  // initialize previous locations of characters
  WarehouseUI ui;
  if (!(ui.check_magic())) {
    printf("Shared memory not initialized\n");
    return 0;
  }
  ui.draw_warehouse();

  // continue looping until main program has quit
  while (!ui.quit()) {
    ui.draw_robots();
    ui.draw_objects();
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }

  return 0;
}