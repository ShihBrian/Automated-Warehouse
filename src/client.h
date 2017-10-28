#ifndef CLIENT_H
#define CLIENT_H

#include <cpen333/process/socket.h>
#include <limits>
#include "server.h"

#define CREATE '1'
#define EDIT '2'
#define PRINT '3'
#define SEND '4'
#define QUIT '5'

std::vector<std::string> product_names = {"","Apples","Bananas","Grapes","Pears","Watermelons"};

enum PRODUCT{
  APPLES = 1,
  BANANAS,
  GRAPES,
  PEARS,
  WATERMELONS,
  DONE
};

void print_menu() {
  std::cout << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "=                ACTIONS                =" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << " (1) Create Order" << std::endl;
  std::cout << " (2) Edit Order" << std::endl;
  std::cout << " (3) Print Order" << std::endl;
  std::cout << " (4) Send Order" << std::endl;
  std::cout << " (5) Quit"  << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "Enter number: ";
  std::cout.flush();
}

void print_products() {
  std::cout << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "=                PRODUCTS               =" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << " (1) Apples" << std::endl;
  std::cout << " (2) Bananas" << std::endl;
  std::cout << " (3) Grapes" << std::endl;
  std::cout << " (4) Pears"  << std::endl;
  std::cout << " (5) Watermelon"  << std::endl;
  std::cout << " (6) Done"  << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "Enter number: ";
  std::cout.flush();
}

struct Order_item{
  std::string product;
  int quantity = 0;
};
#endif //CLIENT_H
