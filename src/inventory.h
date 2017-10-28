#ifndef INVENTORY_H
#define INVENTORY_H

#include <cpen333/process/mutex.h>

struct Shelf {
  int col = 0;
  int row = 0;
  std::string product = "";
  int quantity = 0;
  int weight = 0;
};

class Inventory {
  //TODO: Add thread safety
  std::vector <Shelf> shelves;
  MazeInfo minfo;
  Shelf shelf;
  std::map<std::string,int> total_inv;
  public:

    Inventory(MazeInfo &maze) : minfo(maze) {
      for (int r = 0; r < minfo.rows; r++) {
        for (int c = 0; c < minfo.cols; c++) {
          if (minfo.maze[c][r] == SHELF_CHAR) {
            if (c > 0 && minfo.maze[c - 1][r] == EMPTY_CHAR){
              shelf.col = c-1;
              shelf.row = r;
              shelves.push_back(shelf);
            }
            else if (c < minfo.cols && minfo.maze[c + 1][r] == EMPTY_CHAR){
              shelf.col = c+1;
              shelf.row = r;
              shelves.push_back(shelf);
            }
            else if (r > 0 && minfo.maze[c][r - 1] == EMPTY_CHAR){
              shelf.col = c;
              shelf.row = r-1;
              shelves.push_back(shelf);
            }
            else if (r < minfo.rows && minfo.maze[c][r + 1] == EMPTY_CHAR){
              shelf.col = c;
              shelf.row = r+1;
              shelves.push_back(shelf);
            }
          }
        }
      }
    }

    void display_shelf_loc(){
      for(auto& shelf:shelves){
        std::cout << shelf.col << " " << shelf.row << std::endl;
      }
    }

    void update_inv(std::vector<Order_item> orders,bool add){
        for (auto &order:orders) {
          if(add) total_inv[order.product] += order.quantity;
          else total_inv[order.product] -= order.quantity;
        }
    }

    bool check_stock(std::vector<Order_item> orders){
      for(auto& order:orders){
        if(total_inv[order.product] < order.quantity){
          return false;
        }
      }
      return true;
    }

    void display_total_inv(){
      std::cout << "Total Warehouse Inventory:" << std::endl;
      for(auto const& item : total_inv){
        std::cout << item.first << " : " << item.second << std::endl;
      }
    }

  void get_available_shelf(int& col, int& row, Order_item order){
    for(auto& shelf:shelves){
      if (shelf.quantity == 0){
        shelf.quantity = order.quantity;
        shelf.product = order.product;
        col = shelf.col;
        row = shelf.row;
        break;
      }
    }
  }

  void find_product(int& col, int& row, std::string product){
    for(auto& shelf:shelves){
      if (shelf.product == product){
        col = shelf.col;
        row = shelf.row;
        break;
      }
    }
  }
};


#endif //INVENTORY_H
