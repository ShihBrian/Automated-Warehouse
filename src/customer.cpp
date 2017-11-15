#include "Menu.h"

int main(){
  cpen333::process::socket socket("localhost",55556);
  std::cout << "Client connecting...";
  std::cout.flush();
  if(socket.open()){
    std::cout << "connected." << std::endl;
  }
  Comm comm(socket);
  Order_Menu order_menu;
  int cmd;
  bool quit = false;
  std::vector<Order_item> product_list;
  std::vector<Order_item> Orders;
  std::vector<std::string> products;
  Order_item order;
  order.product = "Apple";
  order.quantity = 10;
  Orders.push_back(order);
  order.product = "Banana";
  order.quantity = 20;
  Orders.push_back(order);

  while(!quit){
    print_menu(customer_menu);
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    switch(cmd){
      case Copt::C_CREATE:
        comm.query_products(product_list, products);
        Orders = order_menu.create_order(products);
        break;
      case Copt::C_SEND:
        comm.send_type(MSG_CUSTOMER);
        comm.send_orders(Orders);
        break;
      case Copt::C_VIEW_INV:
        Orders.clear();
        comm.send_type(MSG_INVENTORY);
        std::cout << "Current Inventory" << std::endl;
        comm.receive_inv(Orders);
        for(auto& order:Orders){
          std::cout << order.product << " " << order.quantity << std::endl;
        }
        Orders.clear();
        break;
      case Copt::C_QUIT:
        quit = true;
        break;
      default:
        std::cout << "Invalid command, try again" << std::endl;
    }
  }

  socket.close();
  return 0;
}