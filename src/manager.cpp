#include "Order.h"
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

void send_type(cpen333::process::socket& socket,MessageType type){
  char buff[1];
  char start = START_BYTE;
  socket.write(&start,1);
  switch(type){
    case MSG_INBOUND:
      buff[0] = MSG_INBOUND;
      socket.write(buff,1);
      break;
    case MSG_ITEM:
      buff[0] = MSG_ITEM;
      socket.write(buff,1);
      break;
    case MSG_END:
      buff[0] = MSG_END;
      socket.write(buff,1);
      break;
    default:
      std::cout << "Invalid message type" << std::endl;
  }
}

void get_size(char* buff, size_t size){
  for (int i=4; i-->0;) {
    // cut off byte and shift size over by 8 bits
    buff[i] = (char)(size & 0xFF);
    size = size >> 8;
  }
}

void send_size(cpen333::process::socket& socket, size_t size){
  char size_buff[4];
  get_size(size_buff,size);
  socket.write(size_buff,4);
}

void send_order(std::vector<Order_item>& Orders,cpen333::process::socket&){
  const char* str;
  char success;
  size_t length;
  send_type(socket,MSG_INBOUND);

  size_t num_items = Orders.size();
  send_size(socket, num_items);

  for(auto& order: Orders){
    send_type(socket,MSG_ITEM);
    send_size(socket,order.quantity);
    std::string product = order.product;
    str = product.c_str();
    length = strlen(str)+1;
    send_size(socket,length);
    socket.write(str,length);
  }
  send_type(socket,MSG_END);

  socket.read_all(&success, 1);
  if(success==SUCCESS_BYTE){
    Orders.clear();
    std::cout << "Server received order\n";
  }
  else if(success==FAIL_BYTE) std::cout << "Server FAILED to receive order\n";
  else std::cout << "Unknown response\n";
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

  Order_item order;
  order.product = "Apples";
  order.quantity = 10;
  Orders.push_back(order);
  order.product = "Bananas";
  order.quantity = 20;
  Orders.push_back(order);

  while(!quit){
    print_menu_manager();
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