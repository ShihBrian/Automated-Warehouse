#ifndef INVENTORY_H
#define INVENTORY_H

#include <cpen333/process/mutex.h>
#include <cpen333/process/shared_memory.h>
#include <map>
#include "SharedData.h"
#include "CircularOrderQueue.h"

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
  std::map<std::string,int> products;
  int weight = 0;
};

class Inventory {
  std::vector <Shelf> shelves;
  WarehouseInfo minfo;
  cpen333::process::shared_object<SharedData> memory_;
  Shelf shelf;
  std::map<std::string,int> total_inv;
  std::vector<std::string> default_products = {"Apple","Banana","Grape","Peach","Watermelon"};
  std::vector<int> default_weight = {2,3,1,2,11};
  std::vector<Order_item> available_products;
  cpen333::process::mutex mutex_;
  int threshold = 0;
  int auto_quantity = 0;
  public:
    Inventory(WarehouseInfo &warehouse) : minfo(warehouse),memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME) {
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

    void update_inv(std::string product, int quantity, bool add){
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(add) total_inv[product] += quantity;
        else total_inv[product] -= quantity;
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

    std::vector<Coordinate> get_available_shelf(Order_item order, int size, int id){
      int row, col, weight, quantity, remaining_weight, iterations, robot_quantity;
      Coordinate coordinate;
      std::vector<Coordinate> coordinates;
      weight = this->get_weight(order.product);
      Coordinate temp = {-1,-1};
      std::vector <Shelf> shelves_backup = shelves;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf:shelves){
          remaining_weight = SHELF_MAX_WEIGHT - shelf.weight;
          if(remaining_weight > weight ) {
            quantity = remaining_weight/weight;
            if(quantity > order.quantity) {
              quantity = order.quantity;
              order.quantity = 0;
            } else {
              order.quantity -= quantity;
            }
            shelf.products[order.product] += quantity;
            shelf.weight += quantity*weight;
            coordinate.col = shelf.robot_col;
            coordinate.row = shelf.robot_row;
            coordinate.product = order.product;
            robot_quantity = ROBOT_MAX_WEIGHT/weight;
            iterations = quantity/robot_quantity;
            for(int i=0;i<iterations;i++) {
              coordinate.quantity = robot_quantity;
              coordinates.push_back(temp);
              coordinates.push_back(coordinate);
            }
            if(iterations == 0) {
              coordinate.quantity = quantity;
              coordinates.push_back(temp);
              coordinates.push_back(coordinate);
            }
            else if(quantity % robot_quantity) {
              coordinate.quantity = quantity % robot_quantity;
              coordinates.push_back(temp);
              coordinates.push_back(coordinate);
            }
            if(order.quantity == 0) break;
          }
        }
        if(memory_->minfo.order_status[1][id] == -1)
          memory_->minfo.order_status[1][id] += coordinates.size()/2 + 1;
        else
          memory_->minfo.order_status[1][id] += coordinates.size()/2;
      }
      if(order.quantity != 0){
        std::cout << "Not enough space" << std::endl;
        shelves = shelves_backup;
        throw 1;
      }
      return coordinates;
    }

    std::vector<Coordinate> get_coordinates(Order_item order, int size, int id){
      int weight = this->get_weight(order.product);
      int robot_quantity = ROBOT_MAX_WEIGHT/weight;
      int quantity;
      Coordinate coordinate;
      std::vector<Coordinate> coordinates;
      Coordinate temp = {-1,-1};

      while(order.quantity > 0) {
        Shelf &s = find_product(order.product);
        {
          std::lock_guard<decltype(mutex_)> lock(mutex_);
          if (s.products[order.product] > order.quantity) {
            s.products[order.product] -= order.quantity;
            s.weight -= order.quantity * weight;
            quantity = order.quantity;
            order.quantity = 0;
          } else {
            order.quantity -= s.products[order.product];
            quantity = s.products[order.product];
            s.products[order.product] = 0;
            s.weight = 0;
          }
        }

        int iterations = quantity/robot_quantity;
        coordinate.col = s.robot_col;
        coordinate.row = s.robot_row;
        coordinate.product = order.product;
        for(int i=0;i<iterations;i++){
          quantity -= robot_quantity;
          coordinate.quantity = robot_quantity;
          coordinates.push_back(coordinate);
          coordinates.push_back(temp);
        }

        if(quantity>0){
          coordinate.quantity = quantity;
          coordinates.push_back(coordinate);
          coordinates.push_back(temp);
        }
      }
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(memory_->minfo.order_status[0][id] == -1)
          memory_->minfo.order_status[0][id] += coordinates.size()/2 + 1;
        else
          memory_->minfo.order_status[0][id] += coordinates.size()/2;
      }
      std::cout << memory_->minfo.order_status[1][id] <<std::endl;
      return coordinates;
    }

    Shelf& find_product(std::string product){
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf:shelves){
          if (shelf.products[product] != 0){
            return shelf;
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

    void remove_inv_item(std::string product){
      int weight = this->get_weight(product);
      for(int i = 0; i<available_products.size();i++){
        if (available_products[i].product == product){
          available_products.erase(available_products.begin()+i);
          //total_inv.erase(product);
        }
      }
    }

    int get_quantity(std::string product){
      return total_inv[product];
    }

    void get_available_products(std::vector<Order_item>& products){
      Order_item item;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& product : available_products){
          item.product = product.product;
          item.weight = product.weight;
          products.push_back(item);
        }
      }
    }

    std::vector<Order_item> get_shelf_info(int col, int row){
      std::vector<Order_item> items;
      Order_item item;
      {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto& shelf: shelves){
          if(shelf.col == col && shelf.row == row){
            for(auto& shelf_item:shelf.products){
              if(shelf_item.second != 0) {
                item.quantity = shelf_item.second;
                item.product = shelf_item.first;
                item.weight = shelf.weight;
                items.push_back(item);
              }
            }
          }
        }
      }
      return items;
    }

  int get_order_id(){
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      for(int i = 0;i<MAX_WAREHOUSE_DOCKS;i++){
        if (memory_->minfo.order_status[0][i] == -1 && memory_->minfo.order_status[1][i] == -1) return i;
      }
    }
    return -1;
  }

  bool check_threshold (std::vector<Order_item>& temp, std::vector<Order_item>& Orders) {
    std::map<std::string,int> inv_dict;
    Order_item order;
    std::map<std::string,int> inv_temp;
    inv_temp = total_inv;
    int quantity;
    for(auto& item: temp){
      inv_temp[item.product] -= item.quantity;
      if(inv_temp[item.product] < 0) inv_temp[item.product] = 0;
    }
    bool restock = false;
    if(threshold > 0) {
      for (auto &product:available_products) {
        inv_dict[product.product] = 0;
      }
      for (auto &item:inv_temp) {
        if (item.first.length() > 1 && inv_dict.find(item.first) != inv_dict.end())
          inv_dict[item.first] = item.second;
      }
      for (auto &item:inv_dict) {
        if (item.second < threshold) {
          quantity = (threshold - item.second) + auto_quantity;
          order.product = item.first;
          order.quantity = quantity;
          Orders.push_back(order);
          restock = true;
        }
      }
    }
    {
      std::lock_guard<decltype(mutex_)> lock(mutex_);
      memory_->minfo.auto_restock = restock;
    }
    return restock;
  }

  void set_auto_restock(int thres, int quantity){
    threshold = thres;
    auto_quantity = quantity;
  }
};

#endif //INVENTORY_H
