#include "ncurses_display.h"
#include "WaitingArea.h"
#include <thread>

int main() {
  
  int width, height, x, y;
//   std::cin >> width >> height >> x >> y;
  width = 20; 
  height = 20;
  x = 20; 
  y = 20; 
  std::shared_ptr<WaitingArea> waitingArea = std::make_shared<WaitingArea>(width, height, x, y); 
  
  NCursesDisplay::Display(std::move(waitingArea), 30);
}