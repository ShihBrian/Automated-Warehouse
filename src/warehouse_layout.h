#ifndef LAB4_MAZE_RUNNER_COMMON_H
#define LAB4_MAZE_RUNNER_COMMON_H

#define MAZE_MEMORY_NAME "lab4_maze_runner"
#define MAZE_MUTEX_NAME "lab4_maze_runner_mutex"
#define DOCKS_SEMAPHORE_NAME "docks_semaphore"

#define WALL_CHAR 'X'
#define EMPTY_CHAR ' '
#define EXIT_CHAR 'E'
#define VISITED_CHAR 'V'
#define HOME_CHAR 'H'
#define SHELF_CHAR 'S'
#define DOCK_CHAR 'D'


#define COL_IDX 0
#define ROW_IDX 1

#define MAX_WAREHOUSE_SIZE 80
#define MAX_RUNNERS   50
#define MAX_WAREHOUSE_DOCKS 10
#define MAGIC 123654

#define MAZE_NAME "C:\\Users\\Brian\\CLionProjects\\Amazoom\\data\\warehouse1.txt"
struct WarehouseInfo {
  int rows;           // rows in warehouse
  int cols;           // columns in warehouse
  char warehouse[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];  // warehouse storage
  char visited[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];
  unsigned dock_col[MAX_WAREHOUSE_DOCKS];
  unsigned dock_row[MAX_WAREHOUSE_DOCKS];
  int curr_dock = 0;
  int num_docks;
};

struct RobotInfo {
  int nrobot;      // number runners
  int rloc[MAX_RUNNERS][2];   // runner locations [col][row]
};

struct SharedData {
  WarehouseInfo minfo;    // warehouse info
  RobotInfo rinfo;  // runner info
  bool quit;         // tell everyone to quit
  int magic;
};

#endif //LAB4_MAZE_RUNNER_COMMON_H
