#ifndef LAB6_ORDER_H
#define LAB6_ORDER_H

#include "server.h"
#include "inventory.h"

/**
 * Basic order information containing a customer id and item id
 */

//TODO: Dynamically add/remove products
std::vector<std::string> customer_menu = {"Create Order","Edit Order","Print Order","Send Order"};
std::vector<std::string> manager_menu = {"Restock","Edit Order","Print Order","Send Order","View Order Status",
                                         "View Inventory","Add New Product","Remove Product"};

enum Copt {
  C_CREATE = 1,
  C_EDIT,
  C_PRINT,
  C_SEND,
  C_QUIT
};

enum Popt {
  M_RESTOCK = 1,
  M_EDIT,
  M_PRINT,
  M_SEND,
  M_VIEW_ORDER_STATUS,
  M_VIEW_INV,
  M_ADD_NEW_PROD,
  M_REMOVE_PROD,
  M_QUIT
};
void print_menu(std::vector<std::string> menu){
  std::cout << std::endl;
  std::cout << "=========================================" << std::endl;
  for(int i=1;i<=menu.size();i++){
    std::cout << " (" << i << ") " << menu[i-1] << std::endl;
  }
  std::cout << " (" << menu.size()+1 << ") Done" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "Enter number: ";
  std::cout.flush();
}

int create_order(std::vector<Order_item>& Orders, std::vector<std::string> products){
  Order_item order;
  int choice,quantity;
  bool valid = false;
  while(true){
    print_menu(products);
    std::cin >> choice;
    if(choice!=products.size()+1) {
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
    order.product = products[choice-1];
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

#endif //LAB6_ORDER_H
