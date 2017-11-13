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
  MSG_PRODUCTS,
  MSG_ADD,
  MSG_REMOVE,
  MSG_SERVER,
  MSG_MOD_ROBOT,
  MSG_SHELF_INFO,
  MSG_QUIT
};

void send_type(cpen333::process::socket& socket,MessageType type){
  char buff = START_BYTE;
  socket.write(&buff,1);
  buff = type;
  socket.write(&buff,1);
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

char rcv_response(cpen333::process::socket& socket, std::string& msg){
  char buff[256];
  char success;
  socket.read_all(&success,1);
  int str_size = get_size(socket);
  socket.read_all(buff,str_size);
  msg = buff;
  return success;
}

//TODO: send generic message
void send_order(std::vector<Order_item>& Orders,cpen333::process::socket& socket){
  const char* str;
  char success;
  size_t length;
  std::string msg;
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

  std::cout << "Send_order: Waiting for response...";
  success = rcv_response(socket,msg);
  std::cout << msg << std::endl;
  if(success==SUCCESS_BYTE) Orders.clear();
}

void send_response(cpen333::process::socket& socket, bool success, std::string msg){
  const char* str;
  size_t length;

  if(success) socket.write(&SUCCESS_BYTE,1);
  else socket.write(&FAIL_BYTE,1);
  str = msg.c_str();
  length = strlen(str)+1;
  send_size(socket,length);
  socket.write(str,length);
}

//TODO: do something with success bool
void send_generic(cpen333::process::socket& socket, char data, bool response){
  char success;
  std::string msg;
  socket.write(&data,1);

  if(response) {
    std::cout << "Send_generic: Waiting for response...";
    success = rcv_response(socket, msg);

    std::cout << msg << std::endl;
  }
}

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
        case MSG_SERVER:
          order_size = get_size(socket);
          std::cout << order_size << std::endl;
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
            send_response(socket,1,"Order successfully received");
            done = true;
          }
          else{
            temp_Orders.clear();
            safe_printf("Recv_inv: Order was not received\n");
            send_response(socket,1,"Order receive failed");
          }
          break;
        default:
          std::cout << "Recv_inv: Invalid msg type" << std::endl;
      }
    }
  }
}

void query_products(std::vector<Order_item>& Orders, cpen333::process::socket& socket, std::vector<std::string>& products){
  Orders.clear();
  send_type(socket,MSG_PRODUCTS);
  receive_inv(socket,Orders);
  for(auto& order:Orders){
    products.push_back(order.product);
  }
}
#endif //COMM_H
