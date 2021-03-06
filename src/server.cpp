#include "server.h"

class Server {
  cpen333::process::shared_object<SharedData> memory;
  cpen333::process::mutex mutex1;
  cpen333::process::socket client;
  cpen333::process::socket_server server;
  order_monitor order_monitor;
  Inventory inv;
  std::vector<Robot*> robots;
  dock_monitor* dock_mon;
  CircularOrderQueue order_queue;
  CircularOrderQueue robot_queue;
  int nrobots = DEFAULT_ROBOTS;
  std::vector<Order_item> temp_orders;
private:
  void modify_robots(bool add, Comm& comm){
    Coordinate poison = {999,999};
    std::vector<Coordinate> order;
    if(add){
      if(nrobots < num_home) {
        nrobots++;
        robots.push_back(new Robot(nrobots, robot_queue, inv));
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
        robot_queue.add(order);
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
      robot_queue.add(order);
    }
    order.push_back({998,998});
    order_queue.add(order);
  }

  void handle_orders(std::vector<Order_item> Orders, Inventory& inv, bool add, bool remove) {
    std::vector<Coordinate> coordinates;
    std::vector<Coordinate> temp;
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
          //list of coordinates the robot must visit in order to fulfil an order
          temp = inv.get_coordinates(order,Orders.size(),order_id);
          for(auto& coord:temp){
            coord.add = add;
            coord.order_id = order_id;
            coordinates.push_back(coord);
          }
        }
        order_queue.add(coordinates);
        coordinates.clear();
        temp_orders = Orders;
        Orders.clear();
        if(!remove)
          add = inv.check_threshold(temp_orders, Orders);
      }
      else std::cout << "Not enough stock" << std::endl;
    }
    if(add){ //restocking
      for (auto &order:Orders) {
        //list of coordinates the robot must visit in order to fulfil an order
        try {
          temp = inv.get_available_shelf(order, Orders.size(), order_id);
        }
        catch(...){
          std::cout << "No more shelf space" << std::endl;
          throw;
        }
        for(auto& coord:temp){
          coord.product = order.product;
          coord.add = add;
          coord.order_id = order_id;
          coordinates.push_back(coord);
        }
      }
      order_queue.add(coordinates);
      coordinates.clear();
    }
  }

  int quit(){
    kill_robots();
    for (auto &robot : robots) {
      robot->join();
    }
    safe_printf("Robots done\n");

    for (auto &robot : robots) {
      delete robot;
      robot = nullptr;
    }

    memory->quit = 1;

    order_monitor.join();
    safe_printf("Order monitor done\n");
    dock_mon->join();
    safe_printf("Dock monitor done\n");
    delete dock_mon;
    dock_mon = nullptr;

    server.close();
    cpen333::pause();
  }
public:
  Server (WarehouseInfo& info, RobotInfo& rinfo) : mutex1(WAREHOUSE_MUTEX_NAME), memory(WAREHOUSE_MEMORY_NAME), inv(info), server(SOCKET_PORT), order_monitor() {
    memory->minfo = info;
    memory->rinfo = rinfo;
    memory->quit = 0;
    memory->magic = MAGIC;
    for (int i = 0; i < MAX_WAREHOUSE_DOCKS; i++) {
      memory->minfo.order_status[0][i] = -1;
      memory->minfo.order_status[1][i] = -1;
    }
    cpen333::process::semaphore dock_semaphore(DOCKS_SEMAPHORE_NAME, memory->minfo.num_docks);
    cpen333::process::semaphore truck_semaphore(TRUCKS_SEMAPHORE_NAME);

    for(int i=0;i<memory->minfo.num_docks;i++){
      truck_semaphore.notify();
    }

    if(num_home < nrobots) nrobots = num_home;
    for (int i = 0; i < nrobots; ++i) {
      robots.push_back(new Robot(i, robot_queue, inv));
    }
    // start everyone
    for (auto &robot : robots) {
      robot->start();
    }

    order_monitor.start();
    inv.init_inv();
    dock_mon = new dock_monitor(order_queue,inv,robot_queue);
    dock_mon->start();
  }

  void service(cpen333::process::socket client, int id, Inventory &inv) {
    std::vector<Order_item> Orders;
    State state = STATE_START;
    Order_item order;
    Comm comm(client);
    std::cout << "Client " << id << " connected" << std::endl;
    bool quit = false;
    bool add = false;
    bool add_product = false;
    bool remove_product = false;
    char msg;
    int str_size, type, order_size;
    char buff[256];
    std::string product;
    Coordinate shelf;
    while (!quit) {
      switch (state) {
        case STATE_START:
          client.read_all(&msg, 1);
          if (msg == START_BYTE) state = STATE_GET_TYPE;
          break;
        case STATE_GET_TYPE:
          client.read_all(&msg, 1);
          type = msg & 0xFF;
          if (type == MSG_CUSTOMER || type == MSG_MANAGER || type == MSG_ADD || type == MSG_REMOVE) {
            order_size = comm.get_size();
            state = STATE_ITEM;
          }
          if (type == MSG_CUSTOMER) add = false;
          else if (type == MSG_MANAGER) add = true;
          else if (type == MSG_ADD) add_product = true;
          else if (type == MSG_REMOVE) remove_product = true;
          else if (type == MSG_INVENTORY) state = STATE_INV;
          else if (type == MSG_PRODUCTS) state = STATE_PRODUCTS;
          else if (type == MSG_SHELF_INFO) state = STATE_SHELF;
          else if (type == MSG_MOD_ROBOT) state = STATE_MOD_ROBOT;
          else if (type == MSG_AUTO) state = STATE_AUTO;
          else if (type == MSG_QUIT) state = STATE_QUIT;
          break;
        case STATE_ITEM:
          client.read_all(&msg, 1);
          client.read_all(&msg, 1);
          type = msg & 0xFF;
          if (type != MSG_ITEM) {
            state = STATE_END;
            break;
          }
          order_size--;
          order.quantity = comm.get_size();
          str_size = comm.get_size();
          client.read_all(buff, str_size);
          product = buff;
          order.product = product;
          order.weight = comm.get_size();
          Orders.push_back(order);
          if (order_size == 0) state = STATE_RECEIVED;
          break;
        case STATE_RECEIVED:
          safe_printf("Order successfully received\n");
          if (add_product) {
            add_product = false;
            inv.add_new_item(Orders[0].product, Orders[0].quantity);
            comm.send_response(1, "Adding new product to inventory");
          } else if (remove_product) {
            remove_product = false;
            int num = inv.get_quantity(Orders[0].product);
            if(num) {
              Orders[0].quantity = num;
              handle_orders(Orders,inv,false, true);
            }
            inv.remove_inv_item(Orders[0].product);
            comm.send_response(1, "Removing product from inventory");
          } else {
            if(!add){
              if (inv.check_stock(Orders))
                comm.send_response(1, "Delivery truck arrived, waiting to be loaded...");
              else
                comm.send_response(0, "Not enough inventory to fulfill order");
            }
            try {
              handle_orders(Orders, inv, add, false);
              comm.send_response(1, "Restocking truck arrived, unloading...");
            }
            catch(...){
              comm.send_response(1, "No more shelf space, order cancelled");
            }
          }
          state = STATE_END;
          break;
        case STATE_END:
          if (order_size != 0) {
            safe_printf("Order was not received\n");
            comm.send_response(1, "Order receive failed");
          }
          Orders.clear();
          state = STATE_START;
          break;
        case STATE_INV:
          Orders.clear();
          inv.get_total_inv(Orders);
          std::cout << "Current Inventory" << std::endl;
          for (auto &order :Orders) {
            std::cout << order.product << " " << order.quantity << std::endl;
          }
          comm.send_type(MSG_SERVER);
          comm.send_orders(Orders);
          state = STATE_START;
          break;
        case STATE_PRODUCTS:
          std::cout << "Getting product list" << std::endl;
          Orders.clear();
          inv.get_available_products(Orders);
          comm.send_type(MSG_SERVER);
          comm.send_orders(Orders);
          state = STATE_START;
          break;
        case STATE_MOD_ROBOT:
          client.read(&msg, 1);
          if (msg == 1) {
            std::cout << "Adding Robot" << std::endl;
            modify_robots(1, comm);
          } else if (msg == 2) {
            std::cout << "Removing Robot" << std::endl;
            modify_robots(0, comm);
          } else {
            std::cout << "Modify robot invalid command" << std::endl;
            comm.send_response(0, "Invalid command");
          }
          state = STATE_START;
          break;
        case STATE_SHELF:
          client.read(&msg, 1);
          shelf.col = msg;
          client.read(&msg, 1);
          shelf.row = msg;
          Orders = inv.get_shelf_info(shelf.col, shelf.row);
          comm.send_type(MSG_SERVER);
          comm.send_orders(Orders);
          Orders.clear();
          state = STATE_START;
          break;
        case STATE_AUTO:
          int thres, quantity;
          client.read(&msg, 1);
          thres = msg;
          client.read(&msg, 1);
          quantity = msg;
          std::cout << "Threshold " << thres << " Quantity: " << quantity << std::endl;
          if (thres >= 0 && quantity >= 0) comm.send_response(1, "Threshold and quantity received");
          else comm.send_response(0, "Invalid threshold or quantity value");
          inv.set_auto_restock(thres, quantity);
          temp_orders = Orders;
          Orders.clear();
          if (inv.check_threshold(temp_orders,Orders))
            handle_orders(Orders, inv, true, false);
          state = STATE_START;
          break;
        case STATE_QUIT:
          quit = true;
          this->quit();
          break;
      }
    }
  }

  void run() {
    server.open();
    std::cout << "Server started on port " << server.port() << std::endl;

    size_t count = 0;

    while (server.accept(client)) {
      std::thread thread(&Server::service, this, std::move(client), count, std::ref(inv));
      thread.detach();
      count++;
    }
  }
};

int main(){
  std::string warehouse = WAREHOUSE_NAME;
  WarehouseInfo info;
  RobotInfo robot_info;
  load_warehouse(warehouse, info);
  init_robots(info, robot_info);
  find_coordinates(info);

  Server server(info,robot_info);
  server.run();
  return 0;
}
