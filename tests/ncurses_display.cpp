/*

Taken from the Udacity CppND from the system monitor project
github repo -> https://github.com/udacity/CppND-System-Monitor

*/

#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include "ncurses_display.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

void NCursesDisplay::DisplaySystem(
    WINDOW *window, bool &doorsAreOpen, int &waitingTime, long & runSim, std::shared_ptr<WaitingArea> waitingArea) {
  int row{0};
  std::string doorsOpenMessage; 
  std::vector<std::shared_ptr<Agent>>  agents; 
  agents = waitingArea->getAgentVector();  

  if (!doorsAreOpen) {
    doorsOpenMessage = "Train arrives in " + to_string((waitingTime - runSim) / 1000) + " seconds."; 
  }
  else {
    doorsOpenMessage = "Train has arrived bitches!"; 
  }


  std::string testString = "x"; 
  mvwprintw(window, ++row, 2, ("Status: " + doorsOpenMessage).c_str());
  mvwprintw(window, ++row, 2, ("Agents in waiting area: " + to_string(agents.size())).c_str());
  mvwprintw(window, ++row, 2, "CPU: ");
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(50.0).c_str());
  wattroff(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(50.0).c_str());
  wattroff(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(30)).c_str());
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(40)).c_str());
  mvwprintw(window, ++row, 2, ("Up Time: " + testString).c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(
    WINDOW *window, std::shared_ptr<WaitingArea> waitingArea, int n,
    bool &doorsAreOpen, int &waitingTime,
    std::chrono::time_point<std::chrono::system_clock> &simStart){
  // wclear(window);
  std:string placeHString = "*";  

  // Fetch agent's grid
  std::vector<std::vector<int>> grid = waitingArea->getAgentGrid(doorsAreOpen, waitingTime, simStart); 

  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{16};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  // Modified due to issues not updating properyl
  // Reference https://knowledge.udacity.com/questions/160777
  // wclear(window);
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));

  int rowCounter = 3; 
  int colCounter = 3;
  int colorDoor = (!doorsAreOpen) ? 3 : 7;
  std::string rowString;

  for (auto k : grid) {
    rowCounter++;
    colCounter = 3;
    for (auto l : k) {
      colCounter = colCounter + 2;
      if (l == 0) {
        rowString = ". ";
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
      } else if (l == 1) {
        wattron(window, COLOR_PAIR(4)); 
        rowString = "A ";
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(4)); 

      } else if (l == 2) {
        wattron(window, COLOR_PAIR(colorDoor)); 
        rowString = "X";
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(colorDoor)); 
      }
    }
  }
}

void NCursesDisplay::Display(std::shared_ptr<WaitingArea> waitingArea, int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW* process_window =
      newwin(3 + n, x_max - 1, system_window->_maxy + 1, 0);
    
  // start simulation
  waitingArea->simulate(); 
  
  // Simulation starting time (approx)
  std::chrono::time_point<std::chrono::system_clock> simStart;
  simStart = std::chrono::system_clock::now();
  bool doorsAreOpen = false; 
  int waitingTime = 20000; // Time until train arrival

  while (1) {
    std::vector<std::vector<int>> grid = waitingArea->getAgentGrid(doorsAreOpen, waitingTime, simStart); 
    long runSim = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now() - simStart)
                        .count();

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_WHITE); 
    wclear(process_window);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system_window, doorsAreOpen, waitingTime, runSim, waitingArea); 
    DisplayProcesses(process_window, waitingArea, n, doorsAreOpen, waitingTime, simStart); 
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
  endwin();
}