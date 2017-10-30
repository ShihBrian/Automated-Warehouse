#ifndef COMM_H
#define COMM_H

#include "server.h"

static const char START_BYTE = 0xEE;
static const char SUCCESS_BYTE = 0xFF;
static const char FAIL_BYTE = 0xFE;

enum MessageType {
  MSG_CUSTOMER,
  MSG_MANAGER,
  MSG_ORDER,
  MSG_ITEM,
  MSG_END,
  MSG_INVENTORY,
  MSG_QUIT
};

void send_type(cpen333::process::socket& socket,MessageType type){
  char buff[1];
  char start = START_BYTE;
  socket.write(&start,1);
  switch(type){
    case MSG_CUSTOMER:
      buff[0] = MSG_CUSTOMER;
      socket.write(buff,1);
      break;
    case MSG_MANAGER:
      buff[0] = MSG_MANAGER;
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
    case MSG_INVENTORY:
      buff[0] = MSG_INVENTORY;
      socket.write(buff,1);
      break;
    default:
      std::cout << "Invalid message type " << type << std::endl;
  }
}

int get_size(cpen333::process::socket& client){
  char size_buff[4];
  client.read_all(size_buff, 4);
  return (size_buff[0] << 24) | (size_buff[1] << 16) | (size_buff[2] << 8) | (size_buff[3] & 0xFF);
}

void get_send_size(char* buff, size_t size){
  for (int i=4; i-->0;) {
    // cut off byte and shift size over by 8 bits
    buff[i] = (char)(size & 0xFF);
    size = size >> 8;
  }
}

void send_size(cpen333::process::socket& socket, size_t size){
  char size_buff[4];
  get_send_size(size_buff,size);
  socket.write(size_buff,4);
}

void send_order(std::vector<Order_item>& Orders,cpen333::process::socket& socket,bool customer){
  const char* str;
  char success;
  size_t length;
  if(customer) send_type(socket,MSG_CUSTOMER);
  else send_type(socket,MSG_MANAGER);

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

  std::cout << "Waiting for response...";
  socket.read_all(&success, 1);
  if(success==SUCCESS_BYTE){
    Orders.clear();
    std::cout << "Server received order\n";
  }
  else if(success==FAIL_BYTE) std::cout << "Server FAILED to receive order\n";
  else std::cout << "Unknown response\n";
}



#endif //COMM_H
