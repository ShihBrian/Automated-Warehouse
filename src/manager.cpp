#include "Order.h"
#include "comm.h"

void receive_inv(cpen333::process::socket& socket,  std::vector<Order_item>& Orders){
  std::vector<Order_item> temp_Orders;
  Order_item order;
  bool done = false;
  char msg;
  int order_size;
  int str_size;
  char buff[256];
  std::string product;
  while(!done) {
    socket.read_all(&msg, 1);
    if(msg==START_BYTE) {
      socket.read_all(&msg, 1);
      int type = msg & 0xFF;
      switch (type) {
        case MSG_CUSTOMER:
          order_size = get_size(socket);
          break;
        case MSG_MANAGER:
          order_size = get_size(socket);
          break;
        case MSG_ITEM:
          order_size--;
          order.quantity = get_size(socket);
          str_size = get_size(socket);
          socket.read_all(buff,str_size);
          product = buff;
          order.product = product;
          temp_Orders.push_back(order);
          break;
        case MSG_END:
          if(order_size == 0){
            Orders = temp_Orders;
            temp_Orders.clear();
            socket.write(&SUCCESS_BYTE,1);
            done = true;
          }
          else{
            temp_Orders.clear();
            safe_printf("Order was not received\n");
            socket.write(&FAIL_BYTE,1);
          }
          break;
        default:
          std::cout << "Invalid msg type" << std::endl;
      }
    }
  }
}

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
    std::cout << "Cmd: " << cmd << std::endl;
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
        Orders.clear();
        send_type(socket,MSG_INVENTORY);
        std::cout << "Current Inventory" << std::endl;
        receive_inv(socket,Orders);
        for(auto& order:Orders){
          std::cout << order.product << " " << order.quantity << std::endl;
        }
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