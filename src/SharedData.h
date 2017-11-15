#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "Constants.h"

struct WarehouseInfo {
  int rows;           // rows in warehouse
  int cols;           // columns in warehouse
  char warehouse[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];  // warehouse storage
  char visited[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];
  unsigned dock_col[MAX_WAREHOUSE_DOCKS];
  unsigned dock_row[MAX_WAREHOUSE_DOCKS];
  int curr_dock = 0;
  int num_docks;
  unsigned restock = 0;
  unsigned deliver = 0;
  int order_status[2][MAX_WAREHOUSE_DOCKS];
};

struct RobotInfo {
  int nrobot;      // number runners
  int rloc[MAX_ROBOTS][2];   // runner locations [col][row]
  int busy[MAX_ROBOTS];
  int task[MAX_ROBOTS];
  char product[MAX_ROBOTS][MAX_WORD_LENGTH];
  int quantity[MAX_ROBOTS];
  int dest[MAX_ROBOTS][2];
  int home[MAX_ROBOTS];
  int dock[MAX_ROBOTS];
};

struct SharedData {
  WarehouseInfo minfo;    // warehouse info
  RobotInfo rinfo;  // runner info
  bool quit;         // tell everyone to quit
  int magic;
};

#endif //SHAREDDATA_H
