#ifndef COMM_H
#define COMM_H

#include "server.h"

static const char START_BYTE = 0xEE;
static const char SUCCESS_BYTE = 0xFF;
static const char FAIL_BYTE = 0xFE;

enum MessageType {
  MSG_CUSTOMER,
  MSG_MANAGER,
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

enum State {
  STATE_START,
  STATE_GET_TYPE,
  STATE_ITEM,
  STATE_RECEIVED,
  STATE_INV,
  STATE_PRODUCTS,
  STATE_SHELF,
  STATE_MOD_ROBOT,
  STATE_END,
  STATE_QUIT
};

class Comm {
  cpen333::process::socket& socket;

  private:
    void get_send_size(char *buff, size_t size) {
      for (int i = 4; i-- > 0;) {
        // cut off byte and shift size over by 8 bits
        buff[i] = (char) (size & 0xFF);
        size = size >> 8;
      }
    }

    void send_size(size_t size) {
      char size_buff[4];
      get_send_size(size_buff, size);
      socket.write(size_buff, 4);
    }

    void send_order(Order_item order) {
      const char *str;
      size_t length;
      send_type(MSG_ITEM);
      send_size(order.quantity);
      std::string product = order.product;
      str = product.c_str();
      length = strlen(str) + 1;
      send_size(length);
      socket.write(str, length);
    }
  public:
    Comm(cpen333::process::socket& socket) : socket(socket) {};

    void send_type(MessageType type) {
      char buff = START_BYTE;
      socket.write(&buff, 1);
      buff = type;
      socket.write(&buff, 1);
    }

    int get_size() {
      char size_buff[4];
      socket.read_all(size_buff, 4);
      return (size_buff[0] << 24) | (size_buff[1] << 16) | (size_buff[2] << 8) | (size_buff[3] & 0xFF);
    }

    char rcv_response(std::string &msg) {
      char buff[256];
      char success;
      socket.read_all(&success, 1);
      int str_size = get_size();
      socket.read_all(buff, str_size);
      msg = buff;
      return success;
    }

    void send_orders(std::vector<Order_item> &Orders) {
      char success;
      std::string msg;
      size_t num_items = Orders.size();
      send_size(num_items);

      for (auto &order: Orders) {
        send_order(order);
      }
      send_type(MSG_END);

      std::cout << "Orders sent, waiting for response...";
      success = rcv_response(msg);
      std::cout << msg << std::endl;
      if (success == SUCCESS_BYTE) Orders.clear();
    }

    void send_single_order(Order_item order){
      std::string msg;
      send_size(1);
      send_order(order);
      send_type(MSG_END);

      std::cout << "Orders sent, waiting for response...";
      rcv_response(msg);
      std::cout << msg << std::endl;
    }

    void send_response(bool success, std::string msg) {
      const char *str;
      size_t length;

      if (success) socket.write(&SUCCESS_BYTE, 1);
      else socket.write(&FAIL_BYTE, 1);
      str = msg.c_str();
      length = strlen(str) + 1;
      send_size(length);
      socket.write(str, length);
    }

    void send_int(char data, bool response) {
      char success;
      std::string msg;
      socket.write(&data, 1);

      if (response) {
        std::cout << "Send_int: Waiting for response...";
        success = rcv_response( msg);

        std::cout << msg << std::endl;
      }
    }

    void receive_inv(std::vector<Order_item> &Orders) {
      std::vector<Order_item> temp_Orders;
      Order_item order;
      bool done = false;
      char msg;
      int order_size;
      int str_size;
      char buff[256];
      std::string product;
      while (!done) {
        socket.read_all(&msg, 1);
        if (msg == START_BYTE) {
          socket.read_all(&msg, 1);
          int type = msg & 0xFF;
          switch (type) {
            case MSG_SERVER:
              order_size = get_size();
              break;
            case MSG_ITEM:
              order_size--;
              order.quantity = get_size();
              str_size = get_size();
              socket.read_all(buff, str_size);
              product = buff;
              order.product = product;
              temp_Orders.push_back(order);
              break;
            case MSG_END:
              if (order_size == 0) {
                Orders = temp_Orders;
                temp_Orders.clear();
                send_response(1, "Order successfully received");
                done = true;
              } else {
                temp_Orders.clear();
                safe_printf("Recv_inv: Order was not received\n");
                send_response(1, "Order receive failed");
              }
              break;
            default:
              std::cout << "Recv_inv: Invalid msg type" << std::endl;
          }
        }
      }
    }

    void query_products(std::vector<Order_item> &Orders, std::vector<std::string> &products) {
      products.clear();
      Orders.clear();
      send_type(MSG_PRODUCTS);
      receive_inv(Orders);
      for (auto &order:Orders) {
        products.push_back(order.product);
      }
    }

    Order_item get_shelf_info(int col, int row){
      std::vector<Order_item> Orders;
      send_type(MSG_SHELF_INFO);
      send_int(col, false);
      send_int(row, false);
      receive_inv(Orders);
      return Orders[0];
    }
};
#endif //COMM_H
