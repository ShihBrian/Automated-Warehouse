#include "Order.h"
#include "comm.h"

int main(){
  cpen333::process::socket socket("localhost",55555);
  std::cout << "Client connecting...";
  std::cout.flush();

  if(socket.open()){
    std::cout << "connected." << std::endl;
  }

  int cmd;
  bool quit = false;
  std::vector<Order_item> Orders;

  Order_item order;
  order.product = "Apples";
  order.quantity = 10;
  Orders.push_back(order);
  order.product = "Bananas";
  order.quantity = 20;
  Orders.push_back(order);

  //TODO: View order status
  while(!quit){
    print_menu(manager_menu);
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    switch(cmd){
      case Popt::M_RESTOCK:
        create_order(Orders);
        break;
      case Popt::M_EDIT:
        edit_order(Orders);
        break;
      case Popt::M_PRINT:
        print_order(Orders);
        break;
      case Popt::M_SEND:
        send_order(Orders,socket,false);
        break;
      case Popt::M_VIEW_ORDER_STATUS:
        break;
      case Popt::M_VIEW_INV:
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