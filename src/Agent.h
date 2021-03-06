#ifndef AGENT_H
#define AGENT_H

#include "WaitingArea.h"
#include <mutex>
#include <thread>

// directional deltas
const int delta[4][2]{{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

// Agent positional deffinitions
struct AgentPosition {
public:
  enum Constants {
    GOAL = 189,
    CLOSED = 1,
    NOT_VISITED = 0,
    FOUND_PATH = 3,
    CURRENT_AGENT = 6,
    NOT_VISITED_F = 188  // Only for the terminal formatting
  };

  static std::string terminalCode(int const position) {
    switch (position) {
    case GOAL:
      return "X ";
    case FOUND_PATH:
      return "* ";
    case CLOSED:
      return "* ";
    case CURRENT_AGENT:
      return "A ";
    default:
      return ". ";
    }
  }
};

/*
Agent class declaration
*/
class Agent : public std::enable_shared_from_this<Agent> {
public:
  Agent();
  Agent(std::shared_ptr<GridCell> currentPosition,
        std::vector<std::shared_ptr<GridCell>> &cells,
        std::shared_ptr<bool> &openDoor, int x_goal, int y_goal, bool exitAgent,
        int id);
  void setCurrentPosition(std::shared_ptr<GridCell> position);
  void walk();
  void moveToValidCell();
 
  void setGridCells(std::vector<std::shared_ptr<GridCell>> cells); 
  void setCurrentGrid();
  void arrivedToDestination(){ _arrivedDestination=true; }
  int getUnitsUntilGoal(); 
  std::shared_ptr<Agent> get_shared_this() { return shared_from_this(); }
  std::vector<std::vector<int>> getAStarPath() { return _aStarPath; }
  void setAStarPath(std::vector<std::vector<int>> currentGrid); 

  // Related to A* search
  void cellSort(std::vector<std::vector<double>> *v);
  bool checkValidCell(int x, int y); 
  double calculateHeuristic(int x_current, int y_current); 
  void addToOpen(double x, double y, double g, double h); 
  void expandNeighbors(std::vector<double> &current);
  int getXGoal() { return _x_goal; } 
  int getYGoal() { return _y_goal; }
  int getAgentID() { return _id; }
  bool isExitAgent() { return _exitAgent; } 
  std::tuple<int, int> getCurrentCoordinates() { return _currentCoords; } 
  void setCurrentCoordinates(std::tuple<int, int> currentCoords); 
  void Search(); 

private:
  void move();
  int _x_goal; 
  int _y_goal; 
  bool _exitAgent; 
  std::shared_ptr<GridCell> _currentPosition;
  double _speed;
  std::vector<std::shared_ptr<GridCell>> _cells;
  std::vector<std::vector<double>> _openList; 
  std::vector<std::vector<int>> _currentGrid; 
  bool _arrivedDestination; 
  std::shared_ptr<bool> _openDoor; 
  int _unitsTilGoal;
  std::vector<std::vector<int>> _aStarPath;
  std::tuple<int, int> _currentCoords; 
  int _id; 
};

#endif