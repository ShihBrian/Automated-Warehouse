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

/**
 * Randomly places all possible maze runners on an empty
 * square in the maze
 * @param minfo maze input
 * @param rinfo runner info to populate
 */
void init_runners(const MazeInfo& minfo, RunnerInfo& rinfo) {
  rinfo.nrunners = 0;
  // fill in random placements for future runners
  for (size_t i=0; i<MAX_RUNNERS; ++i) {
    rinfo.rloc[i][COL_IDX] = 1;
    rinfo.rloc[i][ROW_IDX] = 18;
  }
}

/**
 * Main function to run the restaurant
 * @return
 */
int main(int argc, char* argv[]) {
  // read maze from command-line, default to maze0
  std::string maze = MAZE_NAME;
  if (argc > 1) {
    maze = argv[1];
  }
  cpen333::process::shared_object<SharedData> memory(MAZE_MEMORY_NAME);
  cpen333::process::mutex mutex1(MAZE_MUTEX_NAME);
  MazeInfo info;
  RunnerInfo runners;
  load_maze(maze, info);
  init_runners(info, runners);
  {
    std::lock_guard<decltype(mutex1)> lock(mutex1);
    memory->minfo = info;
    memory->rinfo = runners;
    memory->quit = 0;
    memory->magic = MAGIC;
  }

  // bunch of robots, clients and servers
  std::vector<Robot*> robots;
  std::vector<Client*> clients;
  const int nrobots = 4;
  const int nclients = 1;
  CircularOrderQueue order_queue;
  CircularOrderQueue serve_queue;

  for (int i=0; i<nrobots; ++i) {
    robots.push_back(new Robot(i, order_queue, serve_queue));
  }
  for (int i=0; i<nclients; ++i) {
    clients.push_back(new Client(i, order_queue));
  }
  // start everyone
  for (auto& chef : robots) {
    chef->start();
  }
  for (auto& client : clients) {
    client->start();
  }
  for (auto& client : clients) {
    client->join();
  }
  safe_printf("Client done\n");
  for(int i=0;i<nrobots;i++){
    order_queue.add({999,999});
  }
  for (auto& robot : robots) {
    robot->join();
  }
  safe_printf("Robots done\n");


  // free all memory
  for (auto& client : clients) {
    delete client;
    client = nullptr;
  }
  for (auto& robot : robots) {
    delete robot;
    robot = nullptr;
  }

  cpen333::pause();

  return 0;
}