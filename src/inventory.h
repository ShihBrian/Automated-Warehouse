#ifndef INVENTORY_H
#define INVENTORY_H

#include <cpen333/process/mutex.h>

#define SHELF_MAX_WEIGHT 200
#define ROBOT_MAX_WEIGHT 50

struct Order_item{
  std::string product;
  int quantity = 0;
  int weight = 0;
};

struct Shelf {
  int col = 0;
  int row = 0;
  int robot_col = 0;
  int robot_row = 0;
  std::string product = "";
  int quantity = 0;
  int weight = 0;
};

//TODO: function to return shelves to visit to fulfill order, opposite of get available shelves
class Inventory {
  std::vector <Shelf> shelves;
  WarehouseInfo minfo;
  Shelf shelf;
  std::map<std::string,int> total_inv;
  std::vector<std::string> default_products = {"Apple","Banana","Grape","Pear","Watermelon"};
  std::vector<int> default_weight = {2,3,1,2,10};
  std::vector<Order_item> available_products;
  cpen333::process::mutex mutex_;
  public:
    Inventory(WarehouseInfo &maze) : minfo(maze), mutex_(MAZE_MUTEX_NAME) {
      for (int r = 0; r < minfo.rows; r++) {
        for (int c = 0; c < minfo.cols; c++) {
          if (minfo.warehouse[c][r] == SHELF_CHAR) {
            if (c > 0 && minfo.warehouse[c - 1][r] == EMPTY_CHAR){
              shelf.robot_col = c-1;
              shelf.robot_row = r;
            }
            else if (c < minfo.cols && minfo.warehouse[c + 1][r] == EMPTY_CHAR){
              shelf.robot_col = c+1;
              shelf.robot_row = r;
            }
            else if (r > 0 && minfo.warehouse[c][r - 1] == EMPTY_CHAR){
              shelf.robot_col = c;
              shelf.robot_row = r-1;
            }
            else if (r < minfo.rows && minfo.warehouse[c][r + 1] == EMPTY_CHAR){
              shelf.robot_col = c;
              shelf.robot_row = r+1;
            }
            shelf.col = c;
            shelf.row = r;
            shelves.push_back(shelf);
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
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for (auto &order:orders) {
          if(add) total_inv[order.product] += order.quantity;
          else total_inv[order.product] -= order.quantity;
        }
      }
    }

    bool check_stock(std::vector<Order_item> orders){
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& order:orders){
          if(total_inv[order.product] < order.quantity){
            return false;
          }
        }
        return true;
      }
    }

    void get_total_inv(std::vector<Order_item>& orders){
      Order_item order;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto const& item : total_inv){
          order.product = item.first;
          order.quantity = item.second;
          orders.push_back(order);
        }
      }
    }

    std::vector<Coordinate> get_available_shelf(Order_item order){
      int row, col, weight, quantity, remaining_weight, iterations;
      Coordinate coordinate;
      std::vector<Coordinate> coordinates;
      weight = get_weight(order.product);
      Coordinate temp = {-1,-1};
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf:shelves){
          if (shelf.product == order.product || shelf.quantity == 0){
            remaining_weight = SHELF_MAX_WEIGHT - shelf.weight;
            if(remaining_weight > weight ) {
              quantity = remaining_weight/weight;
              if(quantity > order.quantity) {
                quantity = order.quantity;
                order.quantity = 0;
              } else {
                order.quantity -= quantity;
              }
              shelf.quantity += quantity;
              shelf.product = order.product;
              shelf.weight += quantity*weight;
              coordinate.col = shelf.robot_col;
              coordinate.row = shelf.robot_row;

              iterations = std::ceil((quantity*weight)/((double)ROBOT_MAX_WEIGHT));
              for(int i=0;i<iterations;i++) {
                coordinates.push_back(temp);
                coordinates.push_back(coordinate);
              }
              if(order.quantity == 0) break;
            }
          }
        }
      }
      return coordinates;
    }

    void find_product(int& col, int& row, std::string product){
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf:shelves){
          if (shelf.product == product){
            col = shelf.robot_col;
            row = shelf.robot_row;
            break;
          }
        }
      }
    }

    void init_inv(){
      Order_item item;
      for(int i=0;i<5;i++){
        item.product = default_products[i];
        item.weight = default_weight[i];
        available_products.push_back(item);
      }
    }

    int get_weight(std::string product){
      for(auto& item: available_products){
        if (item.product == product) return item.weight;
      }
      return 0;
    }

    void add_new_item(std::string product, int weight){
      Order_item item;
      item.product = product;
      item.weight = weight;
      available_products.push_back(item);
    }

    int remove_inv_item(std::string product){
      for(int i = 0; i<available_products.size();i++){
        if (available_products[i].product == product){
          available_products.erase(available_products.begin()+i);
          return 1;
        }
      }
      return 0;
    }

    void get_available_products(std::vector<Order_item>& products){
      Order_item item;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& product : available_products){
          item.product = product.product;
          products.push_back(item);
        }
      }
    }

    Order_item get_shelf_info(int col, int row){
      Order_item item;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf: shelves){
          if(shelf.col == col && shelf.row == row){
            item.quantity = shelf.quantity;
            item.product = shelf.product;
            item.weight = shelf.weight;
            return item;
          }
        }
      }
      item.product = "N/A";
      item.quantity = 0;
      item.weight = 0;
      return item;
    }

};


#endif //INVENTORY_H
