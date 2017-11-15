#ifndef MENU_H
#define MENU_H

#include "server.h"
#include "inventory.h"

const std::vector<std::string> customer_menu = {"Create Order","Send Order","View Inventory"};
const std::vector<std::string> manager_menu = {"Restock","Send Restocking Truck", "View Inventory","Add New Product", "Remove Product","Add/Remove Robots","Show Shelf Info"};
const std::vector<std::string> mod_robot_menu = {"Add Robot", "Remove Robot"};

enum Copt {
  C_CREATE = 1,
  C_SEND,
  C_VIEW_INV,
  C_QUIT
};

enum Popt {
  M_RESTOCK = 1,
  M_SEND,
  M_VIEW_INV,
  M_ADD_NEW_PROD,
  M_REMOVE_PROD,
  M_MOD_ROBOT,
  M_SHELF_INFO,
  M_QUIT
};

class Order_Menu {
  std::vector<Order_item> Orders;
  std::map<std::string,int> order_dict;

  void print_order(){
    std::cout << std::endl << "Current Orders:" << std::endl;
    for(auto& item: order_dict){
      std::cout << item.first << " : " << item.second << std::endl;
    }
  }

  public:
    Order_Menu () : Orders(), order_dict() {}

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

    std::vector<Order_item> create_order(std::vector<std::string> products){
      Order_item order;
      Orders.clear();
      order_dict.clear();
      int choice,quantity;
      while(true){
        this->print_order();
        std::cout << std::endl << "Enter negative quantity to subtract product";
        print_menu(products);
        std::cin >> choice;
        if(choice<products.size()+1 && choice > 0) {
          std::cout << "Quantity: " << std::endl;
          std::cin >> quantity;
          while(std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(256,'\n');
            std::cout << "Quantity must be an integer, try again" << std::endl;
            std::cin >> quantity;
          }
          order_dict[products[choice-1]] += quantity;
          if(order_dict[products[choice-1]]<0) order_dict[products[choice-1]] = 0;
        }
        else if(choice == products.size()+1){
          for(auto& item: order_dict){
            if(item.second != 0) {
              order.product = item.first;
              order.quantity = item.second;
              Orders.push_back(order);
            }
          }
          return Orders;
        }
        else std::cout << "Invalid product, choose a different one" << std::endl;

      }
    }
};

#endif //MENU_H
