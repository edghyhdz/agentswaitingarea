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
  return this->_currentAgent->getAgentID();
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
  // Total of agents displayed is agentNumber * 2
  int agentNumber = 20;

  // Get exit cell id
  int exitID = (width * (_y_exit - 1) + _x_exit) - 1;

  std::shared_ptr<GridCell> init_grid, init_grid_exit;

  // Initialize agents on grid
  for (int i = 0; i < agentNumber; ++i) {
    init_grid = _cells.at(0);
    init_grid_exit = _cells.at(exitID); 
    
    std::shared_ptr<Agent> agent = std::make_shared<Agent>(
        init_grid_exit, _cells, _openDoors, 1, 1, true, i);
    init_grid_exit->updateCell(agent);
    _agents.emplace_back(agent);
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
  _threads.front().join();
  std::for_each(_threads.begin(), _threads.end(),
                [](std::thread &t) { t.join(); });
  std::for_each(_threadAgents.begin(), _threadAgents.end(),
                [](std::thread &t) { t.join(); });

}

void WaitingArea::simulate() {
  for (auto &a : _agents) {
    _threadAgents.emplace_back(std::thread(&Agent::walk, a));
  }
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
  int x, y, agentCode;
  std::vector<std::vector<int>> a_path;
  for (auto &cell : _cells) {
    std::tuple<int, int> tmp = cell->getCoordinates();
    x = std::get<0>(tmp);
    y = std::get<1>(tmp);

    if (cell->cellIsTaken() == true) {
      grid[y][x] = cell->getCurrentAgentID(); // Give agent ID
    } else if (y == this->_y_exit - 1 && x == this->_x_exit - 1) {
      grid[y][x] = AgentPosition::GOAL;  // Exit from waitingArea
    } else {
      grid[y][x] = AgentPosition::NOT_VISITED_F;  // Other unvisited cell from grid
    }
  }
  return grid; 
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
          tempRow.push_back(AgentPosition::CURRENT_AGENT);  // TODO: Change to struct
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
