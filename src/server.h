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
#include "inventory.h"

Coordinate home;

void load_maze(const std::string& filename, WarehouseInfo& minfo) {

  // initialize number of rows and columns
  minfo.rows = 0;
  minfo.cols = 0;

  std::ifstream fin(filename);
  std::string line;

  // read warehouse file
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
          minfo.warehouse[col][row] = line[col];
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

void init_runners(const WarehouseInfo& minfo, RobotInfo& rinfo) {
  rinfo.nrobot = 0;

  for(int col = 0; col < minfo.cols; col++) {
    for (int row = 0; row < minfo.rows; row++) {
      if(minfo.warehouse[col][row] == 'H'){
        home.col = col;
        home.row = row;
      }
    }
  }

  for (size_t i=0; i<MAX_ROBOTS; ++i) {
    rinfo.rloc[i][COL_IDX] = home.col;
    rinfo.rloc[i][ROW_IDX] = home.row;
  }
}

#endif //SERVER_H
