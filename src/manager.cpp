#include "Manager.h"

int main(){
  cpen333::process::socket socket("localhost",55556);
  cout << "Client connecting...";
  cout.flush();

  if(socket.open()){
    cout << "connected." << endl;
  }
  Comm comm(socket);
  Manager manager(comm);

  bool quit = false;
  int cmd;

  while(!quit){
    cmd = manager.get_menu_option();
    switch(cmd){
      case Popt::M_RESTOCK:
        manager.create_order();
        break;
      case Popt::M_SEND:
        manager.send_order();
        break;
      case Popt::M_VIEW_INV:
        manager.view_inv();
        break;
      case Popt::M_ADD_NEW_PROD:
        manager.add_new_product();
        break;
      case Popt::M_REMOVE_PROD:
        manager.remove_product();
        break;
      case Popt::M_MOD_ROBOT:
        manager.modify_robot();
        break;
      case Popt::M_SHELF_INFO:
        manager.get_shelf_info();
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