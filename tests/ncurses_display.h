#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include "Agent.h"
#include "WaitingArea.h"
#include <curses.h>
#include <iostream>
#include <string>
#include <thread>
// #include "process.h"
// #include "system.h"

namespace NCursesDisplay {
void Display(std::shared_ptr<WaitingArea> waitingArea, int n = 10);
void DisplayStats(WINDOW *window, bool &doorsAreOpen, int &waitingTime,
                   long &runSim, std::shared_ptr<WaitingArea> waitingArea);
void DisplayAllAgents(
    WINDOW *window, std::shared_ptr<WaitingArea> waitingArea, int n,
    bool &doorsAreOpen, int &waitingTime,
    std::chrono::time_point<std::chrono::system_clock> &simStart,
    int agentNumber);
void DisplayAStarPath(WINDOW *window, int n, std::shared_ptr<WaitingArea> waitingArea, int agentNumber);
std::string ProgressBar(float percent);
}; // namespace NCursesDisplay

#endif