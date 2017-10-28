#ifndef SERVER_H
#define SERVER_H

#include <cpen333/process/socket.h>
#include <fstream>
#include <thread>
#include <random>
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <map>
#include "robot.h"
#include "CircularOrderQueue.h"
#include "warehouse_layout.h"


int end_col, end_row;

void load_maze(const std::string& filename, MazeInfo& minfo) {

  // initialize number of rows and columns
  minfo.rows = 0;
  minfo.cols = 0;

  std::ifstream fin(filename);
  std::string line;

  // read maze file
  if (fin.is_open()) {
    int row = 0;  // zeroeth row
    while (std::getline(fin, line)) {
      int cols = line.length();
      if (cols > 0) {
        // longest row defines columns
        if (cols > minfo.cols) {
          minfo.cols = cols;
        }
        for (size_t col=0; col<cols; ++col) {
          minfo.maze[col][row] = line[col];
          minfo.visited[col][row] = 0;
        }
        ++row;
      }
    }
    minfo.rows = row;
    fin.close();
  }
  else printf("Unable to open file\n");

}

//TODO: Find home position
void init_runners(const MazeInfo& minfo, RunnerInfo& rinfo) {
  rinfo.nrunners = 0;
  // fill in random placements for future runners
  for (size_t i=0; i<MAX_RUNNERS; ++i) {
    rinfo.rloc[i][COL_IDX] = 1;
    rinfo.rloc[i][ROW_IDX] = 18;
  }
}

void find_shelves(std::vector<std::pair<int,int>>& shelves, MazeInfo& minfo){
  for(int r = 0; r<minfo.rows;r++){
    for(int c = 0; c<minfo.cols;c++){
      if(minfo.maze[c][r] == SHELF_CHAR){
        if(c > 0 && minfo.maze[c-1][r] == EMPTY_CHAR) shelves.push_back({c-1,r});
        else if(c < minfo.cols && minfo.maze[c+1][r] == EMPTY_CHAR) shelves.push_back({c+1,r});
        else if(r > 0 && minfo.maze[c][r-1] == EMPTY_CHAR) shelves.push_back({c,r-1});
        else if(r < minfo.rows && minfo.maze[c][r+1] == EMPTY_CHAR) shelves.push_back({c,r+1});
      }
    }
  }
}

#endif //SERVER_H
