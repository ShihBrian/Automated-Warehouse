#ifndef CLIENT_H
#define CLIENT_H

#include "Menu.h"

using namespace std;

class Client {
  protected:
  Comm comm;
  vector<Order_item> product_list;
  vector<string> product_names;
  Order_Menu order_menu;
  vector<Order_item> Orders;
  int cmd;
  public:

    Client(Comm& comm) : comm(comm) {}

    virtual int get_menu_option(){ std::cout << "Error, no menu for base class" << std::endl; return -1;}

    virtual void send_order(){}

    void create_order(){
      comm.query_products(product_list,product_names);
      Orders = order_menu.create_order(product_names);
    }

    void view_inv(){
      Orders.clear();
      comm.send_type(MSG_INVENTORY);
      cout << "Current Inventory" << endl;
      comm.receive_inv(Orders);
      for(auto& order:Orders){
        cout << order.product << " " << order.quantity << endl;
      }
      Orders.clear();
    }
};

#endif //_CLIENT_H
