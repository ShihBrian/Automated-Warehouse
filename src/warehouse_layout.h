#ifndef LAB4_MAZE_RUNNER_COMMON_H
#define LAB4_MAZE_RUNNER_COMMON_H

#define MAZE_MEMORY_NAME "lab4_maze_runner"
#define MAZE_MUTEX_NAME "lab4_maze_runner_mutex"

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

#define MAGIC 123654

#define MAZE_NAME "C:\\Users\\Brian\\CLionProjects\\Amazoom\\data\\warehouse1.txt"
struct WarehouseInfo {
  int rows;           // rows in maze
  int cols;           // columns in maze
  char maze[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];  // maze storage
  char visited[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];
};

struct RunnerInfo {
  int nrunners;      // number runners
  int rloc[MAX_RUNNERS][2];   // runner locations [col][row]
};

struct SharedData {
  WarehouseInfo minfo;    // maze info
  RunnerInfo rinfo;  // runner info
  bool quit;         // tell everyone to quit
  int magic;
};

#endif //LAB4_MAZE_RUNNER_COMMON_H
