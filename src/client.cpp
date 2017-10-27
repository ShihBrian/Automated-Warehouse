#include "client.h"

int create_order(std::vector<Order_item>& Orders){
  Order_item order;
  int choice,quantity;
  bool valid = false;
  while(true){
    print_products();
    std::cin >> choice;
    if(choice!=DONE) {
      while(!valid) {
        std::cout << "Quantity: " << std::endl;
        std::cin >> quantity;
        if (quantity > 0) valid = true;
        else std::cout << "Quantity must be greater than 0" << std::endl;
      }
    }
    else{
      return 0;
    }
    switch(choice){
      case APPLES:
        order.product = product_names[APPLES];
        break;
      case BANANAS:
        order.product = product_names[BANANAS];
        break;
      case GRAPES:
        order.product = product_names[GRAPES];
        break;
      case PEARS:
        order.product = product_names[PEARS];
        break;
      case WATERMELONS:
        order.product = product_names[WATERMELONS];
        break;
      default:
        std::cout << "Invalid choice, try again" << std::endl;
    }
    order.quantity = quantity;
    Orders.push_back(order);
    valid = false;
  }
}

void print_order(std::vector<Order_item>& Orders){
  std::cout << "Current Order List:" << std::endl;

  for(auto& order: Orders){
    std::cout << order.product << " : " << order.quantity << std::endl;
  }
}

void edit_order(std::vector<Order_item>& Orders){
  bool quit = false;
  int choice, quantity;
  while(!quit) {
    std::cout << "Select an item to edit quantity (0 to delete)." << std::endl;
    for (int i = 0; i < Orders.size(); i++) {
      std::cout << i << ". " << Orders[i].product << " : " << Orders[i].quantity << std::endl;
    }
    std::cout << Orders.size() << ". Done" << std::endl;
    std::cout << "Enter number:";
    std::cin >> choice;

    if(choice >= 0 && choice < Orders.size()){
      std::cout << "New quantity: ";
      std::cin >> quantity;

      if(quantity == 0) Orders.erase(Orders.begin()+choice);
      else Orders[choice].quantity = quantity;
    }
    else if(choice == Orders.size()) quit = true;
    else std::cout << "Invalid choice, try again" << std::endl;
  }
}

void send_order(std::vector<Order_item>& Orders,cpen333::process::socket& socket){
  char buff[256];
  buff[0] = '1';
  socket.write(buff,1);
}

int main(){
  cpen333::process::socket socket("localhost",55555);
  std::cout << "Client connecting...";
  std::cout.flush();

  if(socket.open()){
    std::cout << "connected." << std::endl;
  }

  char cmd;
  bool quit = false;
  std::vector<Order_item> Orders;
  while(!quit){
    print_menu();
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    switch(cmd){
      case CREATE:
        create_order(Orders);
        break;
      case EDIT:
        edit_order(Orders);
        break;
      case PRINT:
        print_order(Orders);
        break;
      case SEND:
        send_order(Orders,socket);
        Orders.clear();
        break;
      case QUIT:
        quit = true;
        break;
      default:
        std::cout << "Invalid command, try again" << std::endl;
    }
  }

  socket.close();
  return 0;
}