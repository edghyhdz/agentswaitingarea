#include "WaitingArea.h"
#include "Agent.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>

/*
WaitingAgents member function definitions
*/
// .size method including lock_guard
int WaitingAgents::getSize() {
  std::lock_guard<std::mutex> lock(_mutex);
  return _agents.size();
}

// std::vector::push_back method including lock_guard
void WaitingAgents::pushBack(std::shared_ptr<Agent> agent,
                             std::promise<void> &&promise) {
  std::lock_guard<std::mutex> lock(_mutex);
  _agents.push_back(agent);
  _promises.push_back(std::move(promise));
}

// Permit entry to agent that has waited longer
void WaitingAgents::permitEntryToFirstInQueue() {
  std::lock_guard<std::mutex> lock(_mutex);
  // std::cout << "Agent was granted acces\n"; 

  // get entries from the front of both queues
  auto firstPromise = _promises.begin();
  auto firstAgent = _agents.begin();

  // fulfill promise and send signal back that permission to enter has been
  // granted
  firstPromise->set_value();

  // remove front elements from both queues
  this->_agents.erase(firstAgent);
  this->_promises.erase(firstPromise);

}

/*
GridCell member function definitions
*/
void GridCell::calculateCoordinates() {
  int size = (this->_x * this->_y) - 1;
  int difference = size - this->getID();
  int x, y;

  if (difference < this->_x) {
    x = this->_x - difference;
    y = this->_y;
    _coords = {x - 1, y - 1};
  } else {
    int rem_diff = std::floor(difference / this->_x);
    y = this->_y - rem_diff;
    x = this->_x - (difference - rem_diff * _x);
    _coords = {x - 1, y - 1};
  }
};

// Used only to unlock cell
void GridCell::updateCell() {
  // this->_aPath.clear();
  this->_occupied = false;
  this->_currentAgent = nullptr;
}
void GridCell::updateCell(std::shared_ptr<Agent> agent) {
  // agent->setPreviousPosition(this->get_shared_this());
  this->_occupied = true;
  this->_currentAgent = agent;
}

void GridCell::addAgentToQueue(std::shared_ptr<Agent> agent) {
  std::promise<void> prms;
  std::future<void> ftr = prms.get_future(); 
  _waitingAgents.pushBack(agent, std::move(prms)); 

  // std::cout << "Added agent to exit queue\n"; 
  // Wait until agent is allowed to enter
  ftr.wait(); 

}

void GridCell::processAgentQueue(){
  
  while (true){
    std::this_thread::sleep_for(std::chrono::milliseconds(1)); 

    if (this->_waitingAgents.getSize() > 0 && this->_occupied == false){
      // std::cout << "Agents waiting: " << _waitingAgents.getSize() << std::endl; 
      _waitingAgents.permitEntryToFirstInQueue(); 
    }
  }
}

void GridCell::moveToCell() {}
void GridCell::setAStarPath(std::vector<std::vector<int>> aPath) {
  _aPath = aPath;
};

// Getters
std::tuple<int, int> GridCell::getCoordinates() { return {this->_coords}; }

/*
WaitingArea member class definitions
*/
void WaitingArea::constructArea() {
  int id = 0;
  for (int k = 0; k < this->_height; k++) {
    std::string rows;
    for (int i = 0; i < this->_width; i++) {
      this->_cells.push_back(std::move(
          std::make_shared<GridCell>(id, _width, _height, _x_exit, _y_exit)));
      id++;
      auto temp = this->_cells.back();
      std::tuple<int, int> temp_coords = temp->getCoordinates();
      ;
      // Launch thread to permit entry to exit
      _threads.emplace_back(std::thread(&GridCell::processAgentQueue, temp)); 
      if (_x_exit - 1 == i && _y_exit - 1 == k) {
        rows += "(" + std::to_string(std::get<0>(temp->getCoordinates())) +
                ", " + std::to_string(std::get<1>(temp_coords)) + ") ";
        // Launch thread to permit entry to exit
        // _threads.emplace_back(std::thread(&GridCell::processAgentQueue, temp)); 
        // rows += " X";
      } else {
        rows += "(" + std::to_string(std::get<0>(temp->getCoordinates())) +
                ", " + std::to_string(std::get<1>(temp_coords)) + ") ";
        // rows += " #";
      }
    }
    this->_grid += rows + "\n";
  }
}

// Constructor
WaitingArea::WaitingArea(int width, int height, int x_exit, int y_exit)
    : _width(width), _height(height), _y_exit(y_exit), _x_exit(x_exit) {
  // Constructs area
  this->constructArea();
  int agentNumber = 5;

  for (int i = 0; i < agentNumber; ++i) {
    // Randomly initialize agents on grid
    std::shared_ptr<GridCell> init_grid;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, _cells.size() - 1);
    // init_grid = _cells.at(distr(eng));
    init_grid = _cells.at(i); 
    std::shared_ptr<Agent> agent = std::make_shared<Agent>(init_grid, _cells);
    init_grid->updateCell(agent);
    _agents.emplace_back(agent);
  }
}

WaitingArea::~WaitingArea() {
  // set up thread barrier before this object is destroyed
  std::cout << "Destructor called\n";
  _threads.front().join();
  std::for_each(_threads.begin(), _threads.end(),
                [](std::thread &t) { t.join(); });
}

void WaitingArea::simulate() {

  for (auto &a : _agents) {
    _threads.emplace_back(std::thread(&Agent::walk, a));
  }

  // Print grid
  _threads.emplace_back(std::thread(&WaitingArea::printWaitingArea, this));
}

// Prints grid area
// void WaitingArea::printWaitingArea() { std::cout << _grid; }
void WaitingArea::printWaitingArea() {
  while (true) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::vector<std::vector<int>> grid(this->_height,
                                       std::vector<int>(this->_width));
    int x, y;
    std::vector<std::vector<int>> a_path;
    for (auto &cell : _cells) {
      std::tuple<int, int> tmp = cell->getCoordinates();
      x = std::get<0>(tmp);
      y = std::get<1>(tmp);

      if (cell->cellIsTaken()) {
        // std::cout << "Agent in cell id: " << cell->getID() << std::endl;
        for (auto &c : _cells) {
          if (c->getID() == cell->getID()) {
            a_path = c->getAStartPath();
          }
        }
        grid[y][x] = 1;
      } else if (y == this->_height - 1 && x == this->_width -1 ){
        grid[y][x] = 2;
      }
      else {
        grid[y][x] = 0;
      }
    }
    int colorCode = 30; 
    std::string printGrid;
    for (auto k : grid) {
      std::string row;
      for (auto l : k) {
        if (l == 0) {
          row += "· ";
        } else if (l == 1){
          // row += "A ";
          row += "\033[1;"+ std::to_string(colorCode) + "mA \033[0m";
          colorCode++; 
        }
        else if (l == 2) {
          row += "\033[1;31mX \033[0m";
        }
      }
      printGrid += row + "\n";
    }
    std::cout << "################################################\n";
    std::cout << "\n";
    std::cout << printGrid;
    std::cout << "\n";

    // std::string apath_string;
    // for (auto k : a_path){
    //   std::string rows;
    //   for (auto i : k){
    //     rows += std::to_string(i) + ", ";
    //   }
    //   apath_string += rows + "\n";
    // }
    // std::cout << "################################################\n";
    // std::cout << "\n";
    // std::cout << apath_string;
    // std::cout << "\n";
  }
}

// Prints addresses of each cell from grid
void WaitingArea::printAddresses() {
  for (auto it : this->_cells) {
    std::cout << "Address cell id: " << it->getID() << ", " << it.get()
              << std::endl;
  }
}

// Test stuff
int main() {
  int width, height, x, y;
  std::cin >> width >> height >> x >> y;
  WaitingArea waitingArea = WaitingArea(width, height, x, y);
  waitingArea.simulate();
}