#ifndef AGENT_H
#define AGENT_H

#include "WaitingArea.h"
#include <mutex>
#include <thread>

// directional deltas
const int delta[4][2]{{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

/*
Agent class declaration
*/
class Agent : public std::enable_shared_from_this<Agent> {
public:
  Agent();
  Agent(std::shared_ptr<GridCell> currentPosition, std::vector<std::shared_ptr<GridCell>> &cells, std::shared_ptr<bool> &openDoor); 
  void setCurrentPosition(std::shared_ptr<GridCell> position);
  void walk();
  void moveToValidCell();
  bool checkAgentInCell(); 
  
  void calculatePath();  
  void setGridCells(std::vector<std::shared_ptr<GridCell>> cells); 
  void setCurrentGrid();
  void arrivedToDestination(){ _arrivedDestination=true; }
  int getUnitsUntilGoal(); 
  // void setPreviousPosition(std::shared_ptr<GridCell> position);
  std::shared_ptr<Agent> get_shared_this() { return shared_from_this(); }
  std::vector<std::vector<int>> getAStarPath() { return _aStarPath; }
  void setAStarPath(std::vector<std::vector<int>> currentGrid); 

  // Related to A* search
  // bool compareCells(const std::vector<int> a, const std::vector<int> b);
  void cellSort(std::vector<std::vector<int>> *v);
  bool checkValidCell(int x, int y); 
  // Calculate the manhattan distance
  int calculateHeuristic(int x_current, int y_current); 
  void addToOpen(int x, int y, int g, int h); 
  void expandNeighbors(std::vector<int> &current);
  void Search(); 

private:
  void move();
  std::shared_ptr<GridCell> _currentPosition;
  // std::shared_ptr<GridCell> _previousPosition; 
  double _speed;
  std::vector<std::shared_ptr<GridCell>> _cells;
  std::vector<std::vector<int>> _openList; 
  std::vector<std::vector<int>> _currentGrid; 
  bool _arrivedDestination; 
  std::shared_ptr<bool> _openDoor; 
  int _unitsTilGoal;
  std::vector<std::vector<int>> _aStarPath;
};

#endif