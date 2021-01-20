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
#include <algorithm> 
#include "ncurses_display.h"
#include <fstream>


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

void NCursesDisplay::DisplayStats(
    WINDOW *window, bool &doorsAreOpen, int &waitingTime, long & runSim, std::shared_ptr<WaitingArea> waitingArea) {
  int row{0};
  std::string doorsOpenMessage; 
  std::vector<std::shared_ptr<Agent>>  agents; 
  agents = waitingArea->getAgentVector();  
  long oDoorsTime = 0.0; 

  if (!doorsAreOpen) {
    doorsOpenMessage = "Train arrives in " + to_string((waitingTime - runSim) / 1000) + " seconds."; 
  }
  else {
    doorsOpenMessage = "Train has arrived!";
    oDoorsTime = runSim / 1000; 
  }

  int totalAgents = 0; 
  for (auto cell : waitingArea->getGridVector()){
    if (cell->cellIsTaken()){
      totalAgents++; 
    }
  }

  mvwprintw(window, ++row, 2, ("Status: " + doorsOpenMessage).c_str());
  mvwprintw(window, ++row, 2, ("Doors opened for: " + to_string(oDoorsTime) + " seconds.").c_str());
  mvwprintw(window, ++row, 2, ("Agents simulated: " + to_string(agents.size())).c_str());
  mvwprintw(window, ++row, 2, ("Agents in waiting area: " + to_string(totalAgents)).c_str()); 
  // wattron(window, COLOR_PAIR(2));
  // mvwprintw(window, row, 10, "");
  // wprintw(window, ProgressBar(0.50).c_str());
  // wattroff(window, COLOR_PAIR(2));
  // mvwprintw(window, ++row, 2, "Memory: ");
  // wattron(window, COLOR_PAIR(2));
  // mvwprintw(window, row, 10, "");
  // wprintw(window, ProgressBar(0.50).c_str());
  // wattroff(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, 2,
            ("Agents in waiting area: " + to_string(totalAgents)).c_str()); 
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(40)).c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayAllAgents(
    WINDOW *window, std::shared_ptr<WaitingArea> waitingArea, int n,
    bool &doorsAreOpen, int &waitingTime,
    std::chrono::time_point<std::chrono::system_clock> &simStart, int agentNumber){

  // Fetch agent's grid
  std::vector<std::vector<int>> grid = waitingArea->getAgentGrid(doorsAreOpen, waitingTime, simStart, agentNumber); 

  int row{0};
  int const title_column{2};
  int maxAgentNumber = waitingArea->getAgentVector().size() / 2; 
  int agentNumberExit = agentNumber + maxAgentNumber; 

  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, title_column, "WAITING AREA SIMULATION");
  wattroff(window, COLOR_PAIR(2));

  int rowCounter = 3; 
  int colCounter = 3;
  int colorDoor = (!doorsAreOpen) ? 3 : 7;
  int colorAgent, colorSelected; 
  std::string rowString, agentChar; 

  for (auto k : grid) {
    rowCounter++;
    colCounter = 3;
    for (auto l : k) {
      colCounter = colCounter + 2;
      if (l == 0) {
        rowString = ". ";
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
      } else if (l == 1 || l == 7) {
        colorAgent = (l == 1) ? 4 : 3;
        wattron(window, COLOR_PAIR(colorAgent)); 
        rowString = "A ";
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(colorAgent)); 
      } else if (l == AgentPosition::GOAL) {
        wattron(window, COLOR_PAIR(colorDoor)); 
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(colorDoor)); 
      } else if (l == agentNumber){
        rowString = to_string(l);
        wattron(window, COLOR_PAIR(8)); 
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(8)); 
      } else {
        colorSelected = (l == agentNumber + maxAgentNumber) ? 7: 4; 
        agentChar = (l == agentNumber + maxAgentNumber) ? to_string(l): "A "; 
        wattron(window, COLOR_PAIR(colorSelected)); 
        rowString = agentChar;
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(colorSelected)); 
      }
    }
  }
}

// Takes agent 0 and displays his calculated AStar path
void NCursesDisplay::DisplayAStarPath(WINDOW *window, int n, std::shared_ptr<WaitingArea> waitingArea, int agentNumber){
  int row{0};
  int const title_column{3};
  int const exit_graph_column{5};

  int maxAgentNumber = waitingArea->getAgentVector().size() / 2; 
  int agentNumberExit = agentNumber + maxAgentNumber; 

  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, title_column, "GRAPH STUFF");
  mvwprintw(window, ++++row, exit_graph_column, ("Agent: " + to_string(agentNumber + maxAgentNumber)).c_str());
  mvwprintw(window, row, exit_graph_column + 3 + (maxAgentNumber * 2), ("Agent: " + to_string(agentNumberExit - maxAgentNumber)).c_str());
  wattroff(window, COLOR_PAIR(2));

  std::vector<std::vector<int>> aStarPathEntrance  = waitingArea->getAgentsGrid(agentNumber);
  std::vector<std::vector<int>> aStarPathExit  = waitingArea->getAgentsGrid(agentNumberExit);

  int rowCounter = 3; 
  int colCounter = 3;
  std::string rowString;

  // Draw exit aStarPath 
  for (auto k : aStarPathExit) {
    rowCounter++;
    colCounter = 3;
    for (auto l : k) {
      colCounter = colCounter + 2;
      if (l == AgentPosition::FOUND_PATH) {
        wattron(window, COLOR_PAIR(3));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(3));
      } else if (l == AgentPosition::GOAL) {
        wattron(window, COLOR_PAIR(4));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(4));
      } else if (l == AgentPosition::CURRENT_AGENT) {
        wattron(window, COLOR_PAIR(4));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(4));
      } else if (l == AgentPosition::CLOSED) {
        wattron(window, COLOR_PAIR(1));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
        wattroff(window, COLOR_PAIR(1));
      } else {
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounter, rowString.c_str());
      }
    }
  }

  // Draw entrance aStarPath
  int colCounterR = 3 + colCounter;
  rowCounter = 3; 

  for (auto k : aStarPathEntrance) {
    rowCounter++;
    colCounterR = 3 + colCounter;
    for (auto l : k) {
      colCounterR = colCounterR + 2;
      if (l == AgentPosition::FOUND_PATH) {
        wattron(window, COLOR_PAIR(3)); 
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounterR, rowString.c_str());
        wattroff(window, COLOR_PAIR(3));
      } else if (l == AgentPosition::GOAL) {
        wattron(window, COLOR_PAIR(4));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounterR, rowString.c_str());
        wattroff(window, COLOR_PAIR(4));
      } else if (l == AgentPosition::CLOSED) {
        wattron(window, COLOR_PAIR(1));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounterR, rowString.c_str());
        wattroff(window, COLOR_PAIR(1));
      } else if (l == AgentPosition::CURRENT_AGENT) {
        wattron(window, COLOR_PAIR(4));
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounterR, rowString.c_str());
        wattroff(window, COLOR_PAIR(4));
      } else {
        rowString = AgentPosition::terminalCode(l);
        mvwprintw(window, rowCounter, colCounterR, rowString.c_str());
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
  WINDOW *process_window =
      newwin(3 + n, x_max -( x_max / 2) - 3, system_window->_maxy + 1, 0);
  WINDOW *graph_window =
      newwin(3 + n, x_max - x_max / 2, system_window->_maxy + 1, x_max / 2 - 1);
  keypad(system_window, true); 
  // start simulation
  waitingArea->simulate(); 

  int maxAgentNumber = waitingArea->getAgentVector().size() / 2; 
  int agentNumber = 0;  

  // Simulation starting time (approx)
  std::chrono::time_point<std::chrono::system_clock> simStart;
  simStart = std::chrono::system_clock::now();
  bool doorsAreOpen = false; 
  int waitingTime = 15000; // Time until train arrival
  std::ofstream myfile;

  wtimeout(system_window, 10);
  while (1) {
    switch (wgetch(system_window)) { 
      case KEY_F0 + 1:
        agentNumber += (agentNumber < (maxAgentNumber - 1)) ? 1 : -agentNumber;
        break; 
      case KEY_F0 + 2:
        agentNumber -= (agentNumber > 0) ? 1 : -(maxAgentNumber - 1);
        break; 
    }

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
    init_pair(8, COLOR_WHITE, COLOR_GREEN); 
    wclear(process_window);
    wclear(system_window); 
    wclear(graph_window); 

    box(system_window, 0, 0);
    box(process_window, 0, 0);
    box(graph_window, 0, 0);
    DisplayStats(system_window, doorsAreOpen, waitingTime, runSim, waitingArea); 
    DisplayAllAgents(process_window, waitingArea, n, doorsAreOpen, waitingTime, simStart, agentNumber); 
    DisplayAStarPath(graph_window, n, waitingArea, agentNumber); 
    wrefresh(system_window);
    wrefresh(process_window);
    wrefresh(graph_window); 
    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
  endwin();
}