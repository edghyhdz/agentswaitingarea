#include "Agent.h"
#include <mutex>
#include <random>

Agent::Agent() {
  _currentPosition = nullptr;
  _speed = .5; // blocks per second
}

Agent::Agent(std::shared_ptr<GridCell> position,
             std::vector<std::shared_ptr<GridCell>> &cells) {
  std::cout << "Initialized agent with current position id: "
            << position->getID() << ". Coords: ("
            << std::get<0>(position->getCoordinates()) << ", "
            << std::get<1>(position->getCoordinates()) << ")" << std::endl;
  _cells = cells;
  _currentPosition = position;
  _speed = 0.5;
}

// setters
void Agent::setGridCells(std::vector<std::shared_ptr<GridCell>> cells) {
  _cells = cells;
}

// Walk on grid
void Agent::walk() {
  std::cout << "Started simulation stuff\n";
  bool hasEnteredCellGrid = false;

  // Update every 1 sec
  double cycleDuration = 1000;
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;

  lastUpdate = std::chrono::system_clock::now();

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // compute time difference to stop watch
    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();

    if (timeSinceLastUpdate >= cycleDuration) {
      // Agent should
      // std::vector<std::vector<int>> search_query = this->Search();
      // _openList.clear();

      this->moveToValidCell();
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}

bool Agent::moveToValidCell() {

  std::shared_ptr<GridCell> next_grid;
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(0, _cells.size() - 1);

  this->Search();
  // this->_previousPosition->setAStarPath(_currentGrid);
  // this->_currentPosition->setAStarPath(_currentGrid);
  // print should happen here
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::vector<std::vector<int>> grid(
      this->_currentPosition->getY(),
      std::vector<int>(this->_currentPosition->getX()));

  for (auto &cell : _cells) {
    std::tuple<int, int> tmp = cell->getCoordinates();
    int x_init = std::get<0>(tmp);
    int y_init = std::get<1>(tmp);

    if (cell->cellIsTaken()) {
      grid[y_init][x_init] = 1;
    } else {
      grid[y_init][x_init] = 0;
    }
  }
  std::string printGrid;
  for (auto k : grid) {
    std::string row;
    for (auto l : k) {
      if (l == 0) {
        row += "· ";
      } else if (l == 1) {
        row += "A ";
      }
    }
    printGrid += row + "\n";
  }
  std::cout << "\033[42;31mbold red text\033[0m\n";

  std::cout << "################################################\n";
  std::cout << "\n";
  std::cout << printGrid;
  std::cout << "\n";

  std::tuple<int, int> tmp = this->_currentPosition->getCoordinates(); 
  int x_init = std::get<0>(tmp);
  int y_init = std::get<1>(tmp);

  std::string aPathString;
  for (int i = 0; i < _currentGrid[0].size(); i++) {
    std::string rows;
    for (int k = 0; k < _currentGrid.size(); k++) {
      if (x_init == k && y_init == i){
        rows += "A "; 
      }
      else if (_currentGrid[k][i] == 3) {
        rows += "x ";
      }
      else {
        rows += "· "; 
      }
    }
    aPathString += rows + "\n";
  }
  std::cout << "################################################\n";
  std::cout << "\n";
  std::cout << aPathString;
  std::cout << "\n";

  // here
  _openList.clear();
  this->_currentPosition->updateCell();
  next_grid = _cells.at(distr(eng));
  // Release previous cell
  // update new cell
  next_grid->updateCell(get_shared_this());
  this->setCurrentPosition(next_grid);
};

void Agent::setPreviousPosition(std::shared_ptr<GridCell> position) {
  _previousPosition = position;
}

void Agent::setCurrentPosition(std::shared_ptr<GridCell> position) {
  _currentPosition = position;
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
  sort(v->begin(), v->end(), compareCells);
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