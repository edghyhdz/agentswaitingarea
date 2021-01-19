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

int GridCell::getCurrentAgentID(){

  std::shared_ptr<Agent> agent = this->_currentAgent; 
  

}

bool GridCell::isExitAgent() { return _currentAgent->isExitAgent(); }

// Used only to unlock cell
void GridCell::updateCell() {
  this->_occupied = false;
  this->_currentAgent = nullptr;
}
void GridCell::updateCell(std::shared_ptr<Agent> agent) {
  this->_occupied = true;
  this->_currentAgent = agent;
}

void GridCell::addAgentToQueue(std::shared_ptr<Agent> agent) {
  std::promise<void> prms;
  std::future<void> ftr = prms.get_future(); 
  _waitingAgents.pushBack(agent, std::move(prms)); 
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
      // Launch thread to permit entry to cell
      _threads.emplace_back(std::thread(&GridCell::processAgentQueue, temp)); 
    }
  }
}

void WaitingArea::openDoor(bool open){
  (*this->_openDoors) = open; 
}

// Constructor
WaitingArea::WaitingArea(int width, int height, int x_exit, int y_exit)
    : _width(width), _height(height), _y_exit(y_exit), _x_exit(x_exit) {
  // Constructs area
  this->_openDoors = std::make_shared<bool>(false); 

  this->constructArea();
  int agentNumber = 20;
  std::vector<int> randVector; 

  // Get exit cell id
  int exitID = (width * (_y_exit - 1) + _x_exit) - 1;

  std::shared_ptr<GridCell> init_grid, init_grid_exit;

  for (int i = 0; i < agentNumber; ++i) {
    // Randomly initialize agents on grid
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, _cells.size() - 1);
    int randNumber = distr(eng); 

    init_grid = _cells.at(0);
    init_grid_exit = _cells.at(exitID); 
    
    // init_grid = _cells.at(i);
    std::shared_ptr<Agent> agent = std::make_shared<Agent>(
        init_grid_exit, _cells, _openDoors, 1, 1, true, i);
    init_grid_exit->updateCell(agent);

    // std::shared_ptr<Agent> agent = std::make_shared<Agent>(init_grid, _cells, _openDoors, x_exit, y_exit);
    // init_grid->updateCell(agent);
    _agents.emplace_back(agent);

    // if (!(std::count(randVector.begin(), randVector.end(), randNumber))){
    //   init_grid = _cells.at(randNumber);
    //   // init_grid = _cells.at(i); 
    //   std::shared_ptr<Agent> agent = std::make_shared<Agent>(init_grid, _cells);
    //   init_grid->updateCell(agent);
    //   _agents.emplace_back(agent);

    //   // add to randVector so that position is not reppeated again
    //   randVector.push_back(randNumber); 
    // }
  }

  // Agents going out
  for (int i = 0; i < agentNumber; ++i) {
    init_grid = _cells.at(0);
    std::shared_ptr<Agent> agent = std::make_shared<Agent>(
        init_grid, _cells, _openDoors, x_exit, y_exit, false, i + agentNumber);
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
  std::for_each(_threadAgents.begin(), _threadAgents.end(),
                [](std::thread &t) { t.join(); });

}

void WaitingArea::simulate() {

  for (auto &a : _agents) {
    // _threads.emplace_back(std::thread(&Agent::walk, a));
    _threadAgents.emplace_back(std::thread(&Agent::walk, a));
  }

  // Print grid
  // _threads.emplace_back(std::thread(&WaitingArea::printWaitingArea, this));
}

// Get agent grid
std::vector<std::vector<int>> WaitingArea::getAgentGrid(
    bool &doorsAreOpen, int &waitingTime,
    std::chrono::time_point<std::chrono::system_clock> &simStart, int agentNumber) {

  long runningSim = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now() - simStart)
                        .count();

  // Seconds until train doors are open
  if (runningSim >= waitingTime && doorsAreOpen == false) {
    this->openDoor(true);
    doorsAreOpen = true;

    // Reset simStart to now
    simStart = std::chrono::system_clock::now();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::vector<std::vector<int>> grid(this->_height,
                                      std::vector<int>(this->_width));
  int x, y;
  std::vector<std::vector<int>> a_path;
  for (auto &cell : _cells) {
    std::tuple<int, int> tmp = cell->getCoordinates();
    x = std::get<0>(tmp);
    y = std::get<1>(tmp);

    if (cell->cellIsTaken() == true && cell->isExitAgent() == false) {
      grid[y][x] = 1;
    } else if (cell->cellIsTaken() == true && cell->isExitAgent() == true) {
      grid[y][x] = 7;
    } else if (y == this->_y_exit - 1 && x == this->_x_exit - 1) {
      grid[y][x] = 2;
    } else {
      grid[y][x] = 0;
    }
  }
  return grid; 
}

void WaitingArea::printWaitingArea() {

  // Simulation starting time (approx)
  std::chrono::time_point<std::chrono::system_clock> simStart;
  simStart = std::chrono::system_clock::now();
  bool doorsAreOpen = false; 
  int waitingTime = 5000; // Time until train arrival
  while (true) {

    long runningSim =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - simStart)
            .count();

    // Print train arrival time
    if (doorsAreOpen == false){
      std::cout << "Train arrives in: " << (waitingTime - runningSim)/1000 << " seconds" <<std::endl;
    }
    else{
      std::cout << "Train is here! Opened doors" << std::endl; 
    }
    // Seconds until train doors are open
    if (runningSim >= waitingTime && doorsAreOpen == false) {
      this->openDoor(true); 
      doorsAreOpen = true; 
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
      } else if (y == this->_y_exit - 1 && x == this->_x_exit -1 ){
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

  }
}

std::vector<std::vector<int>> WaitingArea::getAgentsGrid(int agentID) { 
  
  std::shared_ptr<Agent> agent = this->_agents.at(agentID); 
  std::vector<std::vector<int>> grid = agent->getAStarPath();
  std::tuple<int, int> tempCoords = agent->getCurrentCoordinates();
  int x_current = std::get<0>(tempCoords); 
  int y_current = std::get<1>(tempCoords); 

  std::vector<std::vector<int>> aStarPath;

  // Transpose grid
  if (grid.size() > 0) {
    for (int i = 0; i < grid[0].size(); i++) {
      std::vector<int> tempRow;
      for (int k = 0; k < grid.size(); k++) {
        if (x_current == k && i == y_current) {
          tempRow.push_back(6);  // TODO: Change to struct
        } else {
          tempRow.push_back(grid[k][i]);
        }
      }
      aStarPath.push_back(tempRow);
    }
  }

  if (aStarPath.size() <= 0) {
    std::vector<int> dim = this->getConstrArea(); 
    for (int i = 0; i < dim[0]; i++){
      std::vector<int> tempRow;
      for (int k = 0; k < dim[1]; k++){
        tempRow.push_back(0);
      }
      aStarPath.push_back(tempRow);
    }
  }

  return aStarPath; 
}

// Prints addresses of each cell from grid
void WaitingArea::printAddresses() {
  for (auto it : this->_cells) {
    std::cout << "Address cell id: " << it->getID() << ", " << it.get()
              << std::endl;
  }
}

// // Test stuff
// int main() {
//   int width, height, x, y;
//   std::cin >> width >> height >> x >> y;
//   WaitingArea waitingArea = WaitingArea(width, height, x, y);
//   waitingArea.simulate();
// }