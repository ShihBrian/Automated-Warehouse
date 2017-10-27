#include "robot.h"
#include "DynamicOrderQueue.h"
#include "CircularOrderQueue.h"
#include "client.h"
#include "warehouse_layout.h"
#include <string>
#include <fstream>
#include <thread>
#include <random>
#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <map>
#include <cpen333/process/socket.h>

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


void service(OrderQueue& order, cpen333::process::socket client, int id){
  cpen333::process::mutex mutex("Mutex");
  std::cout << "Client " << id << " connected" << std::endl;
  bool valid = true;
  char msg_type;
  while(valid) {
    if (!client.read_all(&msg_type, 1)) {
      valid = false;
      break;
    }
    safe_printf("Received msg type %c\n", msg_type);

    switch(msg_type){
      case '1':
        safe_printf("Add\n");
        break;
      case '2':
        safe_printf("Remove\n");
        break;

    }
  }
}


int main() {
  // read maze from command-line, default to maze0
  std::string maze = MAZE_NAME;

  cpen333::process::shared_object<SharedData> memory(MAZE_MEMORY_NAME);
  cpen333::process::mutex mutex1(MAZE_MUTEX_NAME);
  MazeInfo info;
  RunnerInfo runners;
  load_maze(maze, info);
  init_runners(info, runners);

  memory->minfo = info;
  memory->rinfo = runners;
  memory->quit = 0;
  memory->magic = MAGIC;

  std::vector<std::pair<int,int>> shelves;
  find_shelves(shelves,memory->minfo);
  std::map<std::pair<int,int>,int> inventory;
  std::map<std::pair<int,int>,int>::iterator it;

  std::vector<Robot*> robots;
  const int nrobots = 4;

  CircularOrderQueue order_queue;
  CircularOrderQueue serve_queue;

  for (int i=0; i<nrobots; ++i) {
    robots.push_back(new Robot(i, order_queue, serve_queue));
  }

  // start everyone
  for (auto& robot : robots) {
    robot->start();
  }

  cpen333::process::socket_server server(55555);
  server.open();
  std::cout << "Server started on port " << server.port() << std::endl;

  cpen333::process::socket client;
  size_t count = 0;

  while(server.accept(client)){
    std::thread thread(service,std::ref(order_queue),std::move(client),count);
    thread.detach();
    count++;
  }

  server.close();
/*
  bool quit = false;
  while(!quit){
    char cmd;
    int home_col = 1;
    int home_row = 18;
    while(!quit){
      print_menu();
      std::cin >> cmd;
      std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
      switch(cmd) {
        case '1':
          //get_end();
          for(auto shelf:shelves){
            it = inventory.find(shelf);
            if(it == inventory.end()){
              end_col = shelf.first;
              end_row = shelf.second;
              inventory[shelf] = 1;
              break;
            }
          }
          order_queue.add({end_row,end_col});
          break;
        case '2':
          quit = true;
          break;
        default:
          safe_printf("Invalid cmd entered\n");
      }
    }
  }
*/
  for(int i=0;i<nrobots;i++){
    order_queue.add({999,999});
  }
  for (auto& robot : robots) {
    robot->join();
  }
  safe_printf("Robots done\n");

  for (auto& robot : robots) {
    delete robot;
    robot = nullptr;
  }

  cpen333::pause();

  return 0;
}