#include "Menu.h"

using namespace std;
int main(){
  cpen333::process::socket socket("localhost",55556);
  cout << "Client connecting...";
  cout.flush();

  if(socket.open()){
    cout << "connected." << endl;
  }
  Comm comm(socket);
  Order_Menu order_menu;
  int cmd,col,row;
  bool quit = false;
  vector<Order_item> product_list;
  vector<Order_item> Orders;
  vector<string> products;

  Order_item order;
  order.product = "Apple";
  order.quantity = 10;
  Orders.push_back(order);
  order.product = "Banana";
  order.quantity = 20;
  Orders.push_back(order);

  while(!quit){
    print_menu(manager_menu);
    cin >> cmd;
    cin.ignore (numeric_limits<streamsize>::max(), '\n');
    switch(cmd){
      case Popt::M_RESTOCK:
        comm.query_products(product_list,products);
        Orders = order_menu.create_order(products);
        break;
      case Popt::M_SEND:
        comm.send_type(MSG_MANAGER);
        comm.send_orders(Orders);
        break;
      case Popt::M_VIEW_INV:
        Orders.clear();
        comm.send_type(MSG_INVENTORY);
        cout << "Current Inventory" << endl;
        comm.receive_inv(Orders);
        for(auto& order:Orders){
          cout << order.product << " " << order.quantity << endl;
        }
        Orders.clear();
        break;
      case Popt::M_ADD_NEW_PROD:
        cout << "Enter product name" << endl;
        cin >> order.product;
        cin.ignore (numeric_limits<streamsize>::max(), '\n');
        cout << "Enter weight of item" << endl;
        cin >> order.quantity;
        comm.send_type(MSG_ADD);
        comm.send_single_order(order);
        break;
      case Popt::M_REMOVE_PROD:
        comm.query_products(product_list,products);
        print_menu(products);
        cin >> cmd;
        order.product = products[cmd-1];
        comm.send_type(MSG_REMOVE);
        comm.send_single_order(order);
        break;
      case Popt::M_MOD_ROBOT:
        print_menu(mod_robot_menu);
        cin >> cmd;
        comm.send_type(MSG_MOD_ROBOT);
        comm.send_int(cmd, true);
        break;
      case Popt::M_SHELF_INFO:
        cout << "Enter shelf column: " << endl;
        cin >> col;
        cout << "Enter shelf row: " << endl;
        cin >> row;
        order = comm.get_shelf_info(col,row);
        if(order.product == "N/A") cout << "Not a valid shelf location" << endl;
        else cout << "Product: " << order.product << endl << "Quantity: " << order.quantity << endl;
        break;
      case Popt::M_QUIT:
        quit = true;
        break;
      default:
        cout << "Invalid command, try again" << endl;
    }
  }

  socket.close();
  return 0;
}