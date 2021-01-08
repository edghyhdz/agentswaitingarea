#include "Agent.h"
#include <algorithm>
#include <mutex>
#include <random>

Agent::Agent() {
  _currentPosition = nullptr;
  _speed = .5; // blocks per second
}

Agent::Agent(std::shared_ptr<GridCell> position,
             std::vector<std::shared_ptr<GridCell>> &cells, std::shared_ptr<bool> &openDoor) {
  std::cout << "Initialized agent with current position id: "
            << position->getID() << ". Coords: ("
            << std::get<0>(position->getCoordinates()) << ", "
            << std::get<1>(position->getCoordinates()) << ")" << std::endl;
  _openDoor = openDoor; 
  _cells = cells;
  _arrivedDestination = false; 
  _currentPosition = position;

  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(20, 100);
  std::uniform_int_distribution<> distrUnits(1, 10);
  _unitsTilGoal = distrUnits(eng); 

  _speed = double(distr(eng))/100.0; 
  std::cout << "speed: " << this->_unitsTilGoal<< ", random: "<< double(distr(eng))/100 <<std::endl; 

  // _speed = 0.5;
}

// setters
void Agent::setGridCells(std::vector<std::shared_ptr<GridCell>> cells) {
  _cells = cells;
}

// Walk on grid
void Agent::walk() {
  std::cout << "Started simulation stuff\n";
  bool hasEnteredCellGrid = false;

  // Normal cycle time * agent _speed
  double cycleDuration = 200 / this->_speed;
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;

  lastUpdate = std::chrono::system_clock::now();

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // compute time difference to stop watch
    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();

    if (timeSinceLastUpdate >= cycleDuration) {
      this->moveToValidCell();
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}

// Checks if there is an agent already in that cell to move to
bool Agent::checkAgentInCell(){
  
}

void Agent::setAStarPath(std::vector<std::vector<int>> currentGrid){
  this->_aStarPath = _currentGrid; 
}

void Agent::moveToValidCell() {

  this->Search();

  if (this->_unitsTilGoal>this->getUnitsUntilGoal() && *this->_openDoor == false){
    return; 
  }

  // Only to mark path that agent should follow
  this->_currentPosition->setAStarPath(_currentGrid);
  this->setAStarPath( _currentGrid ); 

  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // here
  _openList.clear();

  std::shared_ptr<GridCell> nextGrid;

  std::tuple<int, int> coordinates = this->_currentPosition->getCoordinates();
  int x_this = std::get<0>(coordinates);
  int y_this = std::get<1>(coordinates);
  bool foundCell = false;
  std::vector<int> validCells;
  for (int i = 0; i < 4; i++) {
    int x_1 = x_this + delta[i][0];
    int y_1 = y_this + delta[i][1];

    // Check first if cells are on grid and if they
    // have value = 3, meaning they are found a* path
    bool on_grid_x = (x_1 >= 0 && x_1 < this->_currentPosition->getX());
    bool on_grid_y = (y_1 >= 0 && y_1 < this->_currentPosition->getY());
    if (on_grid_x && on_grid_y) {
      if (_currentGrid[x_1][y_1] == 3) {
        foundCell = true;
        int cellID = this->_currentPosition->getX() * (y_1 + 1) -
                     (this->_currentPosition->getX() - x_1);
        validCells.push_back(cellID);
      }
      else if (_currentGrid[x_1][y_1] == 5 && this->_arrivedDestination==false && (*this->_openDoor)==true) {
        // Put cell back again until granted access
        std::cout << "Agent arrived to destination\n"; 
        int cellID = this->_currentPosition->getX() * (y_1 + 1) -
                     (this->_currentPosition->getX() - x_1);

        // Get exit Cell ID
        nextGrid = _cells.at(cellID);
        std::chrono::time_point<std::chrono::system_clock> beforeQueue;
        beforeQueue = std::chrono::system_clock::now();
        // Add agent to queue
        nextGrid->addAgentToQueue(this->get_shared_this()); 
        // After agent has been granted access update cell and move to exit
        long afterQueue = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - beforeQueue).count(); 
        std::cout << "Agent was granted acces after: "<< afterQueue << " miliseconds\n"; 
        this->_currentPosition->updateCell();
        this->arrivedToDestination(); 
        break; 
      }
    }
  }

  // Choose next cell randomly from validCells vector
  // Only if we have not arrived yet to destination
  if (foundCell) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(0, validCells.size() - 1);
    nextGrid = _cells.at(validCells.at(distr(eng)));

    nextGrid->addAgentToQueue(this->get_shared_this()); 
    // Remove current cell's agent
    this->_currentPosition->updateCell();
    // Update next grid cell with agent 
    nextGrid->updateCell(this->get_shared_this());
    this->setCurrentPosition(nextGrid);
  }

};

void Agent::setCurrentPosition(std::shared_ptr<GridCell> position) {
  _currentPosition = position;
}

int Agent::getUnitsUntilGoal(){
  int totalUnits = 0; 
  for (auto &i : this->_currentGrid){
    for (auto &k : i){
      if (k == 3) {
        totalUnits++; 
      }
    }
  }
  return totalUnits; 
}

void Agent::setCurrentGrid() {
  int x, y;

  std::vector<std::vector<int>> grid(
      this->_currentPosition->getX(),
      std::vector<int>(this->_currentPosition->getY()));
  _currentGrid = grid;
  for (auto &cell : _cells) {
    std::tuple<int, int> tmp = cell->getCoordinates();
    x = std::get<0>(tmp);
    y = std::get<1>(tmp);
    if (cell->cellIsTaken()) {
      _currentGrid[x][y] = 0;
    } else {
      _currentGrid[x][y] = 0;
    }
  }
}

static bool compareCells(const std::vector<int> a, const std::vector<int> b) {
  int f1 = a[2] + a[3]; // f1 = g1 + h1
  int f2 = b[2] + b[3]; // f2 = g2 + h2
  return f1 > f2;
}
void Agent::cellSort(std::vector<std::vector<int>> *v) {
  std::sort(v->begin(), v->end(), compareCells);
}

bool Agent::checkValidCell(int x, int y) {
  bool on_grid_x = (x >= 0 && x < this->_currentPosition->getX());
  bool on_grid_y = (y >= 0 && y < this->_currentPosition->getY());
  if (on_grid_x && on_grid_y)
    return _currentGrid[x][y] == 0;
  return false;
}

int Agent::calculateHeuristic(int x_current, int y_current) {
  return abs(x_current - this->_currentPosition->getXGoal() - 1) +
         abs(y_current - this->_currentPosition->getYGoal() - 1);
}

void Agent::addToOpen(int x, int y, int g, int h) {
  _openList.push_back(std::vector<int>{x, y, g, h});
  _currentGrid[x][y] = 1; // Closed
}

void Agent::expandNeighbors(std::vector<int> &current) {
  // Get current node's data.
  int x = current[0];
  int y = current[1];
  int g = current[2];

  // Loop through current node's potential neighbors.
  for (int i = 0; i < 4; i++) {
    int x2 = x + delta[i][0];
    int y2 = y + delta[i][1];

    // Check that the potential neighbor's x2 and y2 values are on the grid and
    // not closed.
    if (this->checkValidCell(x2, y2)) {
      // Increment g value and add neighbor to open list.
      int g2 = g + 1;
      int h2 = this->calculateHeuristic(x2, y2);
      this->addToOpen(x2, y2, g2, h2);
    }
  }
}

void Agent::Search() {

  this->setCurrentGrid();

  std::tuple<int, int> coords = this->_currentPosition->getCoordinates();
  int x = std::get<0>(coords);
  int y = std::get<1>(coords);
  int g = 0;
  int h = this->calculateHeuristic(x, y);
  this->addToOpen(x, y, g, h);

  while (_openList.size() > 0) {
    // Get the next node
    this->cellSort(&_openList);
    auto current = _openList.back();
    _openList.pop_back();
    x = current[0];
    y = current[1];
    _currentGrid[x][y] = 3;

    // Check if we're done.
    if (x == this->_currentPosition->getXGoal() - 1 &&
        y == this->_currentPosition->getYGoal() - 1) {
      _currentGrid[std::get<0>(coords)][std::get<1>(coords)] = 0;
      _currentGrid[x][y] = 5;
      break;
    }

    // If we're not done, expand search to current node's neighbors.
    this->expandNeighbors(current);
  }
}

/*
TODO:
Agents can also decide to move somewhere else, if so then they need to update
_waitingList accordingly
- Agents should just move once they are given clearance to move into cell that
is busy at the moment
- Boundary conditions -> where agents spawn from
  - Agents will only spawn if boundary condition is free -> _waitingList
boundary or so
- Print board
  - Use ncursor or so in order not to print all times into console
*/