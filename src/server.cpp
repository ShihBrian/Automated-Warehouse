#include "server.h"

void modify_robots(bool add, Comm& comm){
  Coordinate poison = {999,999};
  std::vector<Coordinate> order;
  if(add){
    if(nrobots < num_home) {
      nrobots++;
      robots.push_back(new Robot(nrobots, incoming_queue));
      robots[robots.size()-1]->start();
      comm.send_response(1,"Successfully added robot");
    }
    else{
      comm.send_response(0,"Already at maximum number of robots");
    }
  }
  else{
    if(nrobots > 0) {
      order.push_back(poison);
      nrobots--;
      incoming_queue.add(order);
      comm.send_response(1,"Successfully removed robot");
    }
    else{
      comm.send_response(0,"There are no robots to remove");
    }
  }
}

void kill_robots(){
  std::vector<Coordinate> order;
  order.push_back({999,999});
  for(int i=0;i<nrobots;i++){
    incoming_queue.add(order);
  }
}

void handle_orders(std::vector<Order_item> Orders, Inventory& inv, bool add) {
  std::vector<Coordinate> coordinates;
  int order_id;
  std::cout << "Incoming orders" << std::endl;
  for(auto& order:Orders){
    std::cout << order.product << " : " << order.quantity << std::endl;
  }
  order_id = inv.get_order_id();
  if(order_id == -1){
    std::cout << "No order_id available" << std::endl;
    return;
  }
  //if removing stock and there is enough
  if (!add) {
    if (inv.check_stock(Orders)) {
      for (auto &order:Orders) {
        coordinates = inv.get_coordinates(order,Orders.size(), order_id);
        coordinates[0].product = order.product;
        coordinates[0].add = 0;
        coordinates[0].order_id = order_id;
        incoming_queue.add(coordinates);
        coordinates.clear();
      }
      inv.update_inv(Orders, add);
    }
    else std::cout << "Not enough stock" << std::endl;
  } else { //restocking
    for (auto &order:Orders) {
      //list of coordinates the robot must visit in order to fulfil an order
      coordinates = inv.get_available_shelf(order,Orders.size(),order_id);
      coordinates[0].product = order.product;
      coordinates[0].add = 1;
      coordinates[0].order_id = order_id;
      incoming_queue.add(coordinates);
      coordinates.clear();
    }
    inv.update_inv(Orders, add);
  }
}

void service(cpen333::process::socket client, int id, Inventory& inv){
  std::vector<Order_item> Orders;
  Order_item order;
  Comm comm(client);
  std::cout << "Client " << id << " connected" << std::endl;
  bool quit = false;
  bool add = false;
  bool add_product = false;
  bool remove_product = false;
  char msg;
  int order_size;
  int str_size;
  char buff[256];
  std::string product;
  Coordinate shelf;
  while(!quit) {
    client.read_all(&msg, 1);
    if(msg==START_BYTE) {
      client.read_all(&msg, 1);
      int type = msg & 0xFF;
      switch (type) {
        case MSG_CUSTOMER:
          add = false;
          order_size = comm.get_size();
          break;
        case MSG_MANAGER:
          add = true;
          order_size = comm.get_size();
          break;
        case MSG_ADD:
          add_product = true;
          order_size = 1;
          break;
        case MSG_REMOVE:
          remove_product = true;
          order_size = 1;
          break;
        case MSG_ITEM:
          order_size--;
          order.quantity = comm.get_size();
          str_size = comm.get_size();
          client.read_all(buff,str_size);
          product = buff;
          order.product = product;
          Orders.push_back(order);
          break;
        case MSG_END:
          if(order_size == 0){
            safe_printf("Order successfully received\n");
            if(add_product) {
              add_product = false;
              inv.add_new_item(Orders[0].product,Orders[0].quantity);
              comm.send_response(1,"Adding new product to inventory");
            }
            else if(remove_product){
              remove_product = false;
              inv.remove_inv_item(Orders[0].product);
              comm.send_response( 1,"Removing product from inventory");
            }
            else{
              if(add) comm.send_response( 1,"Restocking truck arrived, unloading...");
              else {
                if(inv.check_stock(Orders))
                  comm.send_response( 1,"Delivery truck arrived, waiting to be loaded...");
                else
                  comm.send_response( 0,"Not enough inventory to fulfill order");
              }
              handle_orders(Orders,inv,add);
            }
          }
          else{
            safe_printf("Order was not received\n");
            comm.send_response( 1,"Order receive failed");
          }
          Orders.clear();
          break;
        case MSG_INVENTORY:
          Orders.clear();
          inv.get_total_inv(Orders);
          std::cout << "Current Inventory" << std::endl;
          for(auto& order :Orders){
            std::cout << order.product << " " << order.quantity << std::endl;
          }
          comm.send_type( MSG_SERVER);
          comm.send_orders(Orders);
          break;
        case MSG_PRODUCTS:
          Orders.clear();
          inv.get_available_products(Orders);
          comm.send_type( MSG_SERVER);
          comm.send_orders(Orders);
          break;
        case MSG_MOD_ROBOT:
          client.read(&msg,1);
          if(msg == 1) {
            std::cout << "Adding Robot" << std::endl;
            modify_robots(1,comm);
          }
          else if(msg == 2){
            std::cout << "Removing Robot" << std::endl;
            modify_robots(0,comm);
          }
          else{
            std::cout << "Modify robot invalid command" << std::endl;
            comm.send_response( 1,"Invalid command");
          }
          break;
        case MSG_SHELF_INFO:
          client.read(&msg,1);
          shelf.col = msg;
          client.read(&msg,1);
          shelf.row = msg;
          order = inv.get_shelf_info(shelf.col,shelf.row);
          comm.send_type(MSG_SERVER);
          comm.send_single_order(order);
          break;
        case MSG_QUIT:
          quit = true;
          kill_robots();
          break;
      }
    }
  }
}

int main() {
  // read warehouse from command-line, default to maze0
  std::string warehouse = MAZE_NAME;

  cpen333::process::shared_object<SharedData> memory(MAZE_MEMORY_NAME);
  cpen333::process::mutex mutex1(MAZE_MUTEX_NAME);
  WarehouseInfo info;
  RobotInfo robot_info;
  load_warehouse(warehouse, info);
  init_robots(info, robot_info);
  find_coordinates(info);

  memory->minfo = info;
  memory->rinfo = robot_info;
  memory->quit = 0;
  memory->magic = MAGIC;
  for(int i=0;i<MAX_WAREHOUSE_DOCKS;i++){
    memory->minfo.order_status[0][i] = -1;
    memory->minfo.order_status[1][i] = -1;
  }
  num_docks = memory->minfo.num_docks;
  cpen333::process::semaphore dock_semaphore(DOCKS_SEMAPHORE_NAME,num_docks);

  Inventory inv(info);
  inv.init_inv();

  for (int i=0; i<nrobots; ++i) {
    robots.push_back(new Robot(i, incoming_queue));
  }

  // start everyone
  for (auto& robot : robots) {
    robot->start();
  }

  cpen333::process::socket_server server(55556);
  server.open();
  std::cout << "Server started on port " << server.port() << std::endl;

  cpen333::process::socket client;
  size_t count = 0;

  while(server.accept(client)){
    std::thread thread(service,std::move(client),count, std::ref(inv));
    thread.detach();
    count++;
  }

  for (auto& robot : robots) {
    robot->join();
  }

  safe_printf("Robots done\n");

  for (auto& robot : robots) {
    delete robot;
    robot = nullptr;
  }

  server.close();
  cpen333::pause();

  return 0;
}