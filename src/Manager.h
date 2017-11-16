#ifndef MANAGER_H
#define MANAGER_H

#include "Client.h"

enum Popt {
  M_CREATE = 1,
  M_SEND,
  M_VIEW_INV,
  M_ADD_NEW_PROD,
  M_REMOVE_PROD,
  M_MOD_ROBOT,
  M_SHELF_INFO,
  M_RESTOCK,
  M_QUIT
};

class Manager : public Client {
  Order_item order;
  int col,row,threshold,quantity;
public:
  Manager(Comm& comm) : Client(comm) {}

  int get_menu_option(){
    order_menu.print_menu(manager_menu);
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    return cmd;
  }

  void send_order(){
    comm.send_type(MSG_MANAGER);
    comm.send_orders(Orders);
    Orders.clear();
  }

  void add_new_product(){
    cout << "Enter product name" << endl;
    cin >> order.product;
    cin.ignore (numeric_limits<streamsize>::max(), '\n');
    cout << "Enter weight of item" << endl;
    cin >> order.quantity;
    comm.send_type(MSG_ADD);
    comm.send_single_order(order);
  }

  void remove_product() {
    comm.query_products(product_list,product_names);
    order_menu.print_menu(product_names);
    cin >> cmd;
    order.product = product_names[cmd-1];
    comm.send_type(MSG_REMOVE);
    comm.send_single_order(order);
  }

  void modify_robot(){
    order_menu.print_menu(mod_robot_menu);
    cin >> cmd;
    comm.send_type(MSG_MOD_ROBOT);
    comm.send_int(cmd, true);
  }

  void get_shelf_info(){
    cout << "Enter shelf column: " << endl;
    cin >> col;
    cout << "Enter shelf row: " << endl;
    cin >> row;
    order = comm.get_shelf_info(col,row);
    if(order.product == "N/A") cout << "Not a valid shelf location" << endl;
    else cout << "Product: " << order.product << endl << "Quantity: " << order.quantity << endl;
  }

  void restock(){
    comm.send_type(MSG_AUTO);
    cout << "Enter low stock threshold (0 to disable): " << endl;
    cin >> threshold;
    cout << "Enter restock quantity (0 to bring stock up to threshold): " << endl;
    cin >> quantity;
    comm.send_int(threshold,false);
    comm.send_int(quantity,true);
  }
};

#endif //MANAGER_H
