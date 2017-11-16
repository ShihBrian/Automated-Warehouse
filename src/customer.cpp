#include "Customer.h"

int main(){
  cpen333::process::socket socket("localhost",SOCKET_PORT);
  std::cout << "Client connecting...";
  std::cout.flush();
  if(socket.open()){
    std::cout << "connected." << std::endl;
  }

  Comm comm(socket);
  Customer customer(comm);

  int cmd;
  bool quit = false;

  while(!quit){
    cmd = customer.get_menu_option();
    switch(cmd){
      case Copt::C_CREATE:
        customer.create_order();
        break;
      case Copt::C_SEND:
        customer.send_order();
        break;
      case Copt::C_VIEW_INV:
        customer.view_inv(true);
        break;
      case Copt::C_QUIT:
        quit = true;
        break;
      default:
        std::cout << "Invalid command, try again" << std::endl;
    }
  }

  socket.close();
  return 0;
}