#include "Order.h"
#include "comm.h"





int main(){
  cpen333::process::socket socket("localhost",55556);
  std::cout << "Client connecting...";
  std::cout.flush();

  if(socket.open()){
    std::cout << "connected." << std::endl;
  }

  int cmd;
  bool quit = false;
  int weight;
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

  //TODO: View order status
  while(!quit){
    print_menu(manager_menu);
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    switch(cmd){
      case Popt::M_RESTOCK:
        products.clear();
        product_list.clear();
        query_products(product_list,socket,products);
        create_order(Orders,products);
        break;
      case Popt::M_EDIT:
        edit_order(Orders);
        break;
      case Popt::M_PRINT:
        print_order(Orders);
        break;
      case Popt::M_SEND:
        send_type(socket,MSG_MANAGER);
        send_order(Orders,socket);
        break;
      case Popt::M_VIEW_ORDER_STATUS:
        break;
      case Popt::M_VIEW_INV:
        Orders.clear();
        send_type(socket,MSG_INVENTORY);
        std::cout << "Current Inventory" << std::endl;
        receive_inv(socket,Orders);
        for(auto& order:Orders){
          std::cout << order.product << " " << order.quantity << std::endl;
        }
        Orders.clear();
        break;
      case Popt::M_ADD_NEW_PROD:
        std::cout << "Enter product name" << std::endl;
        std::cin >> order.product;
        std::cout << "Enter weight of item" << std::endl;
        std::cin >> order.weight;
        order.quantity = 0;
        Orders.clear();
        Orders.push_back(order);
        send_type(socket,MSG_ADD);
        send_order(Orders,socket);
        break;
      case Popt::M_REMOVE_PROD:
        products.clear();
        product_list.clear();
        query_products(product_list,socket,products);
        print_menu(products);
        std::cin >> cmd;
        Orders.clear();
        order.product = products[cmd-1];
        Orders.push_back(order);
        send_type(socket,MSG_REMOVE);
        send_order(Orders,socket);
        break;
      case Popt::M_MOD_ROBOT:
        print_menu(mod_robot_menu);
        std::cin >> cmd;
        send_type(socket,MSG_MOD_ROBOT);
        send_generic(socket,cmd,"Modify successful","Failed to modify");
        break;
      case Popt::M_QUIT:
        quit = true;
        break;
      default:
        std::cout << "Invalid command, try again" << std::endl;
    }
  }

  socket.close();
  return 0;
}