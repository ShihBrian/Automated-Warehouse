
#include "server.h"

int get_size(cpen333::process::socket& client){
  char size_buff[4];
  client.read_all(size_buff, 4);
  return (size_buff[0] << 24) | (size_buff[1] << 16) | (size_buff[2] << 8) | (size_buff[3] & 0xFF);
}

void service(OrderQueue& orders, cpen333::process::socket client, int id){
  std::vector<Order_item> temp_Orders;
  std::vector<Order_item> Orders;
  Order_item order;
  std::cout << "Client " << id << " connected" << std::endl;
  bool quit = false;
  char msg;
  int order_size;
  int str_size;
  char buff[256];
  std::string product;
  while(!quit) {
    client.read_all(&msg, 1);
    if(msg==START_BYTE) {
      client.read_all(&msg, 1);
      int type = msg & 0xFF;
      switch (type) {
        case MSG_ORDER:
          order_size = get_size(client);
          safe_printf("Order size: %d\n",order_size);
          break;
        case MSG_ADD:
          safe_printf("Add\n");
          break;
        case MSG_ITEM:
          order_size--;
          order.quantity = get_size(client);
          str_size = get_size(client);
          client.read_all(buff,str_size);
          product = buff;
          order.product = product;
          temp_Orders.push_back(order);
          break;
        case MSG_END:
          if(order_size == 0){
            safe_printf("Order successfully received\n");
            Orders = temp_Orders;
            temp_Orders.clear();
          }
          else{
            safe_printf("Order was not received\n");
          }
          break;
        case MSG_QUIT:
          quit = true;
          break;

      }
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