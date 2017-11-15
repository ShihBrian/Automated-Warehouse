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
#include "SharedData.h"
#include "inventory.h"
#include "comm.h"

std::vector<Coordinate> homes;
int num_home = 0;
std::vector<Robot*> robots;
CircularOrderQueue incoming_queue;
int num_docks = 0;
int nrobots = DEFAULT_ROBOTS;

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
  Coordinate home;
  for(int col = 0; col < minfo.cols; col++) {
    for (int row = 0; row < minfo.rows; row++) {
      if(minfo.warehouse[col][row] == 'H'){
        home.col = col;
        home.row = row;
        num_home++;
        homes.push_back(home);
      }
    }
  }
  int count = 0;
  for(auto& home:homes){
    rinfo.rloc[count][COL_IDX] = home.col;
    rinfo.rloc[count][ROW_IDX] = home.row;
    count++;
  }
}

#endif //SERVER_H
