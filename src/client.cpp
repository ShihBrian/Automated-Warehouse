#include <cpen333/process/socket.h>
#include <limits>
void print_menu() {

  std::cout << "=========================================" << std::endl;
  std::cout << "=                  MENU                 =" << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << " (1) Add Song" << std::endl;
  std::cout << " (2) Quit"  << std::endl;
  std::cout << "=========================================" << std::endl;
  std::cout << "Enter number: ";
  std::cout.flush();
}


int main(){
  cpen333::process::socket socket("localhost",55555);
  std::cout << "Client connecting...";
  std::cout.flush();

  if(socket.open()){
    std::cout << "connected." << std::endl;
  }

  char cmd = 0;

  while(cmd != '2'){
    print_menu();
    char buff[256];

    std::cin >> cmd;
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');

    switch(cmd){
      case '1':
        buff[0] = '1';
        socket.write(buff,1);
        break;
      case '2':
        break;
    }
  }
  return 0;
}