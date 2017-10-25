#ifndef CLIENT_H
#define CLIENT_H

#include <cpen333/thread/thread_object.h>
#include <cpen333/thread/semaphore.h>
#include <thread>
#include <vector>

#include "safe_printf.h"

/**
 * Customers place orders into a queue, wait for them to be
 * served, eat, then leave
 */
class Client : public cpen333::thread::thread_object {
  OrderQueue& queue_;
  int id_;
  int end_row = 0;
  int end_col = 0;
 public:
  /**
   * Creates a customer
   * @param id customer id
   * @param menu menu for customer to order food from
   * @param queue queue to place order into
   */
  Client(int id, OrderQueue& queue) :
      id_(id), queue_(queue){}

  /**
   * Unique customer id
   * @return customer id
   */
  int id() {
    return id_;
  }
  void print_menu() {

    safe_printf("=========================================\n");
    safe_printf("=                  MENU                 =\n");
    safe_printf("=========================================\n");
    safe_printf(" 1. Enter destination\n");
    safe_printf(" 2. Go Home\n");
    safe_printf(" 3. Quit\n");
    safe_printf("=========================================\n");
    safe_printf("Enter number: ");
  }

  void get_end(){
    std::string col, row;
    safe_printf("Col: ");
    std::getline(std::cin,col);
    end_col = std::stoi(col);
    safe_printf("\nRow: ");
    std::getline(std::cin,row);
    end_row = std::stoi(row);
  }

  /**
   * Main customer function
   * @return 0 when complete
   */
  int main() {
    char cmd;
    bool quit = false;
    int home_col = 1;
    int home_row = 18;
    while(!quit){
      this->print_menu();
      std::cin >> cmd;
      std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
      switch(cmd) {
        case '1':
          this->get_end();
          queue_.add({end_row,end_col});
          break;
        case '2':
          queue_.add({home_row,home_col});
          break;
        case '3':
          quit = true;
          break;
        default:
          safe_printf("Invalid cmd entered\n");
      }
    }
    return 0;
  }
};

#endif //CLIENT_H
