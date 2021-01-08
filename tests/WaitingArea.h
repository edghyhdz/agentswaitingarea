#ifndef WAITINGAREA_H
#define WAITINGAREA_H

#include <future>
#include <iostream>
#include <memory>
// #include <opencv2/core.hpp>
#include <string>
#include <thread>
#include <vector>

// forward declarations to avoid inlcude cycle
class Agent;

/*
WaitingAgents class declaration
*/
class WaitingAgents {
public:
  // getters / setters
  int getSize();

  // typical behaviour methods
  void pushBack(std::shared_ptr<Agent> agent, std::promise<void> &&promise);
  void permitEntryToFirstInQueue();

private:
  std::vector<std::shared_ptr<Agent>>
      _agents; // list of all agents waiting to enter this cellgrid
  std::vector<std::promise<void>> _promises; // list of associated promises
  std::mutex _mutex;
};

/*
Grid Cell class declaration
*/

class GridCell : public std::enable_shared_from_this<GridCell> {
public:
  GridCell(int id, int x, int y, int x_goal, int y_goal)
      : _id(id), _x(x), _y(y), _x_goal(x_goal), _y_goal(y_goal) {
    this->calculateCoordinates();
    _occupied = false;
  };

  std::tuple<int, int> getCoordinates();
  void calculateCoordinates();
  void updateCell();
  void updateCell(std::shared_ptr<Agent> agent);
  void addAgentToQueue(std::shared_ptr<Agent> agent);
  bool cellIsTaken() { return _occupied; }
  void moveToCell();
  int getID() { return _id; }
  void processAgentQueue();

  int getX() { return _x; }
  int getY() { return _y; }

  int getXGoal() { return _x_goal; }
  int getYGoal() { return _y_goal; }

  std::shared_ptr<GridCell> get_shared_this() { return shared_from_this(); }

  void setAStarPath(std::vector<std::vector<int>> aPath);
  std::vector<std::vector<int>> getAStartPath() { return _aPath; }

  // typical behavior methods
private:
  int _id;
  int _x, _y;                   // widht and height from grid
  int _x_goal, _y_goal;         // end goal coordinates
  std::tuple<int, int> _coords; // grid cell coordinates
  WaitingAgents _waitingAgents; // List of waiting agents to move into cell
  bool _occupied;
  std::shared_ptr<Agent> _currentAgent; // Current agent on cell;
  std::vector<std::vector<int>> _aPath;
};

class WaitingArea {

public:
  // constructor / destructor
  WaitingArea(int width, int height, int x_exit, int y_exit);
  ~WaitingArea();
  // Reference: https://stackoverflow.com/a/23575458/13743493
  WaitingArea(WaitingArea &&o) = default;

  // Getters
  std::vector<std::shared_ptr<GridCell>> getGridVector() { return _cells; }
  std::vector<std::shared_ptr<Agent>> getAgentVector() { return _agents; }

  // Creates waiting area
  void constructArea();
  void openDoor(bool open);
  // Prints waiting area
  std::vector<std::vector<int>>
  getAgentGrid(bool &doorsAreOpen, int &waitingTime,
               std::chrono::time_point<std::chrono::system_clock> &simStart);
  void printWaitingArea();
  void printAddresses();
  void simulate();

private:
  int _width;
  int _height;
  int _y_exit;
  int _x_exit;
  std::vector<std::shared_ptr<GridCell>> _cells;
  std::shared_ptr<bool> _openDoors;
  std::vector<std::shared_ptr<Agent>> _agents;
  std::string _grid;
  std::vector<std::thread> _threads;
};

#endif