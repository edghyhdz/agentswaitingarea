#include "Agent.h"
#include <algorithm>
#include <mutex>
#include <random>

Agent::Agent() {
  _currentPosition = nullptr;
  _speed = .5; // blocks per second
}

Agent::Agent(std::shared_ptr<GridCell> position,
             std::vector<std::shared_ptr<GridCell>> &cells,
             std::shared_ptr<bool> &openDoor, int x_goal, int y_goal, bool exitAgent, int id) {
  std::cout << "Initialized agent with current position id: "
            << position->getID() << ". Coords: ("
            << std::get<0>(position->getCoordinates()) << ", "
            << std::get<1>(position->getCoordinates()) << ")" << std::endl;
  _id = id; 
  _exitAgent = exitAgent; 
  _x_goal = x_goal;  
  _y_goal = y_goal;            
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
      if (this->isExitAgent() == true && (*this->_openDoor) == true) {
        this->moveToValidCell();
      } else if (this->isExitAgent() == false) {
        this->moveToValidCell();
      }
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}

void Agent::setAStarPath(std::vector<std::vector<int>> currentGrid){
  this->_aStarPath = _currentGrid; 
}

void Agent::setCurrentCoordinates(std::tuple<int, int> currentCoords) {
  _currentCoords = currentCoords; 
}; 

void Agent::moveToValidCell() {
  // Every Step, agent calculates path
  this->Search();

  // Adds some randomness to the movement of the agent
  if (this->getUnitsUntilGoal() < 10 &&
      *this->_openDoor == false) {
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
  // Set current coordinates for waitingArea
  this->setCurrentCoordinates(coordinates); 
  int x_this = std::get<0>(coordinates);
  int y_this = std::get<1>(coordinates);
  bool foundCell = false;
  std::vector<int> validCells;
  for (int i = 0; i < 4; i++) {
    int x_1 = x_this + delta[i][0];
    int y_1 = y_this + delta[i][1];


    int cellID = this->_currentPosition->getX() * (y_1 + 1) -
                (this->_currentPosition->getX() - x_1);

    // Check first if cells are on grid and if they
    // have value = FOUND_PATH
    bool on_grid_x = (x_1 >= 0 && x_1 < this->_currentPosition->getX());
    bool on_grid_y = (y_1 >= 0 && y_1 < this->_currentPosition->getY());
    
    // if (on_grid_x && on_grid_y && cellTaken==false) {
    if (on_grid_x && on_grid_y) {

      bool cellTaken = this->_cells.at(cellID)->cellIsTaken();

      if (_currentGrid[x_1][y_1] == AgentPosition::FOUND_PATH &&
          this->_arrivedDestination == false && cellTaken == false) {
        foundCell = true;

        // Vector of valid cells to move to
        validCells.push_back(cellID);
      } else if (_currentGrid[x_1][y_1] == AgentPosition::GOAL &&
                 this->_arrivedDestination == false &&
                 (*this->_openDoor) == true) {
        // Put cell back again until granted access
        std::cout << "Agent arrived to destination" << std::endl; 

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
    // Get closests cell to home -> after doors are opened
    if ((*this->_openDoor)) {
      double x_0, y_0, x_1, y_1, y_diff, x_diff, d_goal;

      x_0 = this->getXGoal() - 1;
      y_0 = this->getYGoal() - 1;

      std::vector<double> distVector;
      for (int &cellID : validCells) {
        std::tuple<int, int> tempCoords = _cells.at(cellID)->getCoordinates();

        x_1 = std::get<0>(tempCoords);
        y_1 = std::get<1>(tempCoords);

        x_diff = std::abs(x_1 - x_0);
        y_diff = std::abs(y_1 - y_0);

        d_goal = std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));
        distVector.push_back(d_goal);
      }

      auto minDist = std::min_element(distVector.begin(), distVector.end());
      int indexMinDist = std::distance(distVector.begin(), minDist);

      nextGrid = _cells.at(validCells.at(indexMinDist));

      nextGrid->addAgentToQueue(this->get_shared_this());
      // Remove current cell's agent
      this->_currentPosition->updateCell();
      // Update next grid cell with agent
      nextGrid->updateCell(this->get_shared_this());
      this->setCurrentPosition(nextGrid);

    } else {

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
  }
};

void Agent::setCurrentPosition(std::shared_ptr<GridCell> position) {
  _currentPosition = position;
}

int Agent::getUnitsUntilGoal(){
  int totalUnits = 0; 
  for (auto &i : this->_currentGrid){
    for (auto &k : i){
      if (k == AgentPosition::FOUND_PATH) {
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
      _currentGrid[x][y] = AgentPosition::NOT_VISITED;
    }
  }
}

static bool compareCells(const std::vector<double> a, const std::vector<double> b) {
  double f1 = a[2] + a[3]; // f1 = g1 + h1
  double f2 = b[2] + b[3]; // f2 = g2 + h2
  return f1 > f2;
}
void Agent::cellSort(std::vector<std::vector<double>> *v) {
  std::sort(v->begin(), v->end(), compareCells);
}

bool Agent::checkValidCell(int x, int y) {
  bool on_grid_x = (x >= 0 && x < this->_currentPosition->getX());
  bool on_grid_y = (y >= 0 && y < this->_currentPosition->getY());
  if (on_grid_x && on_grid_y)
    return _currentGrid[x][y] == AgentPosition::NOT_VISITED; // Non visited gridcell
  return false;
}

double Agent::calculateHeuristic(int x_current, int y_current) {
  // return abs(x_current - this->_currentPosition->getXGoal() - 1) +
  //        abs(y_current - this->_currentPosition->getYGoal() - 1);

  return abs(x_current - this->getXGoal() - 1) +
        abs(y_current - this->getYGoal() - 1);
}

void Agent::addToOpen(double x, double y, double g, double h) {
  _openList.push_back(std::vector<double>{x, y, g, h});
  _currentGrid[x][y] = AgentPosition::CLOSED; // Closed
}

void Agent::expandNeighbors(std::vector<double> &current) {
  // Get current node's data.
  int x = (int)current[0];
  int y = (int)current[1];
  double g = current[2];

  // Loop through current node's potential neighbors.
  for (int i = 0; i < 4; i++) {
    int x2 = x + delta[i][0];
    int y2 = y + delta[i][1];

    // Check that the potential neighbor's x2 and y2 values are on the grid and
    // not closed.
    double multFactor = 1.0;
    if ((*this->_openDoor) && this->isExitAgent() == false) {
      multFactor = 0.5;
    }

    if (this->checkValidCell(x2, y2)) {
      // Increment g value and add neighbor to open list.
      double g2 = g + (1.0 * multFactor); 
      double h2 = this->calculateHeuristic(x2, y2);
      this->addToOpen(x2, y2, g2, h2);
    }
  }
}

void Agent::Search() {

  this->setCurrentGrid();

  std::tuple<int, int> coords = this->_currentPosition->getCoordinates();
  int x = std::get<0>(coords);
  int y = std::get<1>(coords);
  double g = 0;
  double h = this->calculateHeuristic(x, y);
  this->addToOpen(x, y, g, h);

  while (_openList.size() > 0) {
    // Get the next node
    this->cellSort(&_openList);
    auto current = _openList.back();
    _openList.pop_back();
    x = current[0];
    y = current[1];
    _currentGrid[x][y] = AgentPosition::FOUND_PATH;

    // Check if we're done.
    if (x == this->getXGoal() - 1 &&
        y == this->getYGoal() - 1) {
      _currentGrid[std::get<0>(coords)][std::get<1>(coords)] = AgentPosition::NOT_VISITED;
      _currentGrid[x][y] = AgentPosition::GOAL;
      break;
    }

    // If we're not done, expand search to current node's neighbors.
    this->expandNeighbors(current);
  }
}

/*
TODO:
  - ADD MUTEX WHEN CHANGING CELL TO OCCUPIED BY AGENT
*/