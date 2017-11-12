#include "server.h"
#include "Order.h"
#include "comm.h"
#include "inventory.h"

Coordinate home;
std::vector<Robot*> robots;
CircularOrderQueue incoming_queue;
CircularOrderQueue outgoing_queue;
int nrobots = 4;
int num_docks;
//TODO: add manager option to shutdown server
//TODO: semaphore equal to number of docks, robots must wait for available dock
void find_coordinates(WarehouseInfo& info){
  char c;
  Coordinate dock;
  int count = 0;
  for(int col = 0; col < info.cols; col++){
    for(int row = 0; row < info.rows; row++){
      c = info.warehouse[col][row];
      if(c == 'H'){
        home.col = col;
        home.row = row;
      }
      else if(c == 'D'){
        if (c > 0 && info.warehouse[col - 1][row] == EMPTY_CHAR){
          dock.col = col-1;
          dock.row = row;
        }
        else if (col < info.cols && info.warehouse[col + 1][row] == EMPTY_CHAR){
          dock.col = col+1;
          dock.row = row;
        }
        else if (row > 0 && info.warehouse[col][row - 1] == EMPTY_CHAR){
          dock.col = col;
          dock.row = row-1;
        }
        else if (row < info.rows && info.warehouse[col][row + 1] == EMPTY_CHAR){
          dock.col = col;
          dock.row = row+1;
        }
        info.dock_col[count] = dock.col;
        info.dock_row[count] = dock.row;
        count++;
      }
    }
  }
  info.num_docks = count;
}

//TODO: check for no robots or too many
void modify_robots(bool add){
  Coordinate poison = {999,999};
  std::vector<Coordinate> order;
  if(add){
    nrobots++;
    robots.push_back(new Robot(nrobots, incoming_queue, outgoing_queue));
    robots[robots.size()-1]->start();
  }
  else{
    order.push_back(poison);
    nrobots--;
    incoming_queue.add(order);
  }
}

//TODO: add thread safety
void handle_orders(std::vector<Order_item> Orders, Inventory& inv, bool add) {
  int col, row;
  std::vector<Coordinate> coordinates;

  home.col = 1;
  home.row = 18;
  std::cout << "Incoming orders" << std::endl;
  for(auto& order:Orders){
    std::cout << order.product << " : " << order.quantity << std::endl;
  }
  //if removing stock and there is enough
  if (!add) {
    if (inv.check_stock(Orders)) {
      for (auto &order:Orders) {
        inv.find_product(col, row, order.product);
        coordinates.push_back({row,col});
        outgoing_queue.add(coordinates);
        coordinates.clear();
      }
      inv.update_inv(Orders, add);
    }
    else std::cout << "Not enough stock" << std::endl;
  } else { //restocking
    for (auto &order:Orders) {
      //list of coordinates the robot must visit in order to fulfil an order
      coordinates = inv.get_available_shelf(order,home);
      std::cout << "Shelf location for " << order.product << std::endl;
      for(auto& coordinate:coordinates){
        std::cout << coordinate.col << " " << coordinate.row << std::endl;
      }
      incoming_queue.add(coordinates);
      coordinates.clear();
    }
    inv.update_inv(Orders, add);
  }
}

//TODO: Make switch into FSM
void service(cpen333::process::socket client, int id, Inventory& inv){
  std::vector<Order_item> temp_Orders;
  std::vector<Order_item> Orders;
  Order_item order;
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
          order_size = get_size(client);
          break;
        case MSG_MANAGER:
          add = true;
          order_size = get_size(client);
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
            if(add_product) {
              add_product = false;
              inv.add_new_item(Orders[0].product,Orders[0].weight);
              send_response(client,1,"Adding new product to inventory");
            }
            else if(remove_product){
              remove_product = false;
              inv.remove_inv_item(Orders[0].product);
              send_response(client,1,"Removing product from inventory");
            }
            else{
              if(add) send_response(client,1,"Restocking truck arrived, unloading...");
              else send_response(client,1,"Delivery truck arrived, waiting to be loaded...");
              handle_orders(Orders,inv,add);
              Orders.clear();
            }
          }
          else{
            temp_Orders.clear();
            safe_printf("Order was not received\n");
            send_response(client,1,"Order receive failed");
          }
          break;
        case MSG_INVENTORY:
          inv.get_total_inv(temp_Orders);
          send_type(client,MSG_SERVER);
          send_order(temp_Orders,client);
          temp_Orders.clear();
          break;
        case MSG_PRODUCTS:
          inv.get_available_products(temp_Orders);
          send_type(client,MSG_SERVER);
          send_order(temp_Orders,client);
          temp_Orders.clear();
          break;
        case MSG_MOD_ROBOT:
          client.read(&msg,1);
          if(msg == 1) {
            std::cout << "Adding Robot" << std::endl;
            modify_robots(1);
            send_response(client,1,"Successfully added robot");
          }
          else if(msg == 2){
            std::cout << "Removing Robot" << std::endl;
            modify_robots(0);
            send_response(client,1,"Successfully removed robot");
          }
          else{
            std::cout << "Modify robot invalid command" << std::endl;
            send_response(client,1,"Invalid command");
          }
          break;
        case MSG_SHELF_INFO:
          client.read(&msg,1);
          shelf.col = msg;
          client.read(&msg,1);
          shelf.row = msg;
          order = inv.get_shelf_info(shelf.col,shelf.row);
          temp_Orders.clear();
          temp_Orders.push_back(order);
          send_type(client,MSG_SERVER);
          send_order(temp_Orders,client);
          temp_Orders.clear();
          break;
        case MSG_QUIT:
          quit = true;
          break;
      }
    }
  }
}

//TODO: Spawn delivery and restocking trucks
int main() {
  // read warehouse from command-line, default to maze0
  std::string maze = MAZE_NAME;

  cpen333::process::shared_object<SharedData> memory(MAZE_MEMORY_NAME);
  cpen333::process::mutex mutex1(MAZE_MUTEX_NAME);
  WarehouseInfo info;
  RobotInfo runners;
  load_maze(maze, info);
  init_runners(info, runners);
  find_coordinates(info);
  memory->minfo = info;
  memory->rinfo = runners;
  memory->quit = 0;
  memory->magic = MAGIC;


  num_docks = memory->minfo.num_docks;
  cpen333::process::semaphore dock_semaphore(DOCKS_SEMAPHORE_NAME,num_docks);

  Inventory inv(info);
  inv.init_inv();

  //TODO: Add or remove robots dynamically
  for (int i=0; i<nrobots; ++i) {
    robots.push_back(new Robot(i, incoming_queue, outgoing_queue));
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

  server.close();

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