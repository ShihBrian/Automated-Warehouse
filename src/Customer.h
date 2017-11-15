#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Client.h"

class Customer : public Client {
public:
  Customer(Comm& comm) : Client(comm) {}

  int get_menu_option(){
    order_menu.print_menu(customer_menu);
    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    return cmd;
  }

  void send_order(){
    comm.send_type(MSG_CUSTOMER);
    comm.send_orders(Orders);
  }

};

#endif //CUSTOMER_H
