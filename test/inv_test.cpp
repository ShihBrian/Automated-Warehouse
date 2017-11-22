#include "../src/inventory.h"
#include "../src/server.h"
#include "TestException.h"
#include <algorithm>
#include <string>
#include <map>

using namespace std;

void createItem(string product, int quantity,  vector<Order_item>& orders){
  Order_item item;
  item.product = product;
  item.quantity = quantity;
  orders.push_back(item);
}

void testFindProduct(string product, int col, int row, Inventory& inv){
  Shelf shelf = inv.find_product(product);

  if(shelf.col != col || shelf.row != row){
    string msg = "Shelf coordinatres do not match";
    throw TestException(msg);
  }
}

void testGetWeight(string product, int weight, Inventory& inv){
  int w = inv.get_weight(product);
  if(w != weight){
    string msg = "Weight " + to_string(w) + " does not equal " + to_string(weight);
    throw TestException(msg);
  }
}
/*
void testInvUpdate(bool add, Inventory& inv,vector<string> products,vector<int>quantity,vector<Order_item>& orders) {
  Order_item item;
  string msg = "testInvUpdate: ";
  inv.update_inv(orders,add);
  orders.clear();
  inv.get_total_inv(orders);
  for(int i=0;i<orders.size();i++){
    if(orders[i].product != products[i] || orders[i].quantity != quantity[i]){
      msg.append(orders[i].product + " " + to_string(orders[i].quantity) + " does not match expected ");
      msg.append(products[i] + " " + to_string(quantity[i]));
      throw TestException(msg);
    }
  }
  orders.clear();
}
*/

void testGetProducts(Inventory& inv, vector<string> expected_products){
  vector<Order_item> products;
  inv.get_available_products(products);

  for(int i=0;i<products.size();i++){
    if(products[i].product != expected_products[i]) {
      string msg = "Products do not match";
      throw TestException(msg);
    }
  }
}

void testModProducts(Inventory& inv,string product, int weight, vector<string> expected_products,bool add){
  vector<Order_item> products;
  if(add) {
    inv.add_new_item(product, weight);
    testGetWeight(product, weight, inv);
  }
  else
    inv.remove_inv_item(product);

  inv.get_available_products(products);
  testGetProducts(inv,expected_products);
}

/*
void testGetShelfInfo(Inventory& inv, int col, int row, int quantity, string product, int weight){
  Order_item order;
  order = inv.get_shelf_info(col,row);
  if(order.product != product || order.weight != weight || order.quantity != quantity){
    string msg = "Shelf info incorrect";
    throw TestException(msg);
  }
}
*/

void testRestock(Inventory& inv, Order_item order, vector<Coordinate> expected){
  vector<Coordinate> coordinates = inv.get_available_shelf(order, 0,0);
  int count = 0;

  for(auto& coord:coordinates){
    if(coord.col != -1 && coord.row != -1){
      if (coord.col != expected[count].col || coord.row != expected[count].row){
        string msg = "Restock coordinates do not match expected";
        throw TestException(msg);
      }
      count++;
    }
  }
}

int main(){
  string warehouse = TEST_WAREHOUSE_NAME;
  WarehouseInfo info;
  load_warehouse(warehouse, info);
  find_coordinates(info);
  Inventory inv(info);
  inv.init_inv();
  vector<Order_item> orders;
  Order_item order;
  vector<Coordinate> coordinates;
  Coordinate coordinate;
  std::map<std::string,int> products;
  char product[] = "Hello";
  std::string test;

  int count = 0;
  while(product[count] != '\0'){
    test.push_back(product[count]);
    count++;
  }
  products[test] = 10;
  std::printf("%s",test.c_str());
  for(auto& prod:products){
    std::printf("%s %d", prod.first.c_str(),prod.second);
  }
  /*
  try{
    //Add to inventory
    createItem("Apple",10,orders);
    createItem("Banana",20,orders);
    testInvUpdate(true,inv,{"Apple","Banana"},{10,20},orders);

    //Remove from inventory
    createItem("Apple",5,orders);
    createItem("Banana",10,orders);
    testInvUpdate(false,inv,{"Apple","Banana"},{5,10},orders);

    //Get weight of a product
    testGetWeight("Apple",2,inv);

    //Get all available products
    testGetProducts(inv, {"Apple","Banana","Grape","Peach","Watermelon"});

    //Add and remove available product
    testModProducts(inv,"Pen",10,{"Apple","Banana","Grape","Peach","Watermelon","Pen"},true);
    testModProducts(inv,"Apple",0,{"Banana","Grape","Peach","Watermelon","Pen"},false);


    order.product = "Banana";
    order.quantity = 40;

    coordinate.col = 1;
    coordinate.row = 2;
    coordinates.push_back(coordinate);
    coordinates.push_back(coordinate);
    coordinates.push_back(coordinate);
    coordinate.col = 3;
    coordinate.row = 2;
    coordinates.push_back(coordinate);
    //Restocking 40 bananas should split into 4 trips due to shelf and robot weight limits
    testRestock(inv,order,coordinates);

    //Get information about a shelf
    testGetShelfInfo(inv, 2,2, 33, "Banana", 99);

    //Get location of shelf containing a product
    testFindProduct("Banana",2,2,inv);

    std::cout << "All test passed!" << std::endl;
  }catch (TestException& exc) {
    std::cout << exc.what() << std::endl;
  }
*/



  return 0;
}