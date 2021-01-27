# Agent-based model 
`Blue` agents going down towards the `X`, while `orange` angents exiting from the `X` after the `train` has arrived, are directing themselves towards the upper left hand side where the `blue` agents first exited.

<img src="https://github.com/edghyhdz/agentswaitingarea/blob/master/images/terminal.gif" width="700">

## Description
This program runs a concurrent simulation of several agents. They are all dropped into a `waiting area` where they will wonder around until, let's say a train arrives
It consists of a set of `Agents` entering the waiting room, and a set of `Agents` arriving from this mentioned train, and exiting it. 

<img align="left" src="https://media.giphy.com/media/l0HlBO7eyXzSZkJri/giphy.gif" width="200">

The agents are being run on their own `thread`, meaning that they can interact between each other and see at every point in time where another agent is located at. 
Each agent is calculating its own path using `A-star` search algorithm. 

This path is displayed in the right hand side and can be switched to other agents up and down
by pressing `F1` and `F2` respectivelly. 

## Dependencies - (for linux systems only)

 * [ncurses](https://www.gnu.org/software/ncurses/), used for the graphical representation of the agents on the terminal
 
    `sudo apt install libncurses5-dev libncursesw5-dev`
 * [cmake](https://www.gnu.org/software/make/), used to build the executable
    
    `sudo apt-get -y install cmake`
    
## Installation (for linux systems only)

Once all dependencies are installed, 

1. git clone `https://github.com/edghyhdz/agentswaitingarea.git`
2. Inside the root project folder `mkdir build && cd build`
3. `cmake ..`
4. `make`

## Usage

Once installation is complete

1. run `.build/agents_display`
3. The agents entering will be doing so from the upper left hand side,
4. The agents exiting the `train` will be doing so from the lower middle part marked with `X`
2. If you want to check different agent's `a-star` paths, click on `F1` or `F2` to go up or down respectivelly
    * The selected agent will be highlighted in the left hand side of the terminal screen
    * The right hand screen will be displaying

<p>
  <img src="https://github.com/edghyhdz/agentswaitingarea/blob/master/images/left_hand.gif" width="300" />
  <img src="https://github.com/edghyhdz/agentswaitingarea/blob/master/images/right_hand.gif" width="525" /> 
  <br>
  <em>When pressing either F1 or F2, agent is switched and a different a star path is displayed for</em>
  <br>
  <em>both exiting and entering agents. The selected agents are highlighted in different colors.</em>
</p>


## References
Some code snippet, references or help were taken from the following repositories, 

* [CppND - System monitor project](https://github.com/udacity/CppND-System-Monitor-Project-Updated.git), 
where several code snippets were used for the terminal user interface

* From the [CppND - Concurrent traffic simulation project](https://github.com/udacity/CppND-Program-a-Concurrent-Traffic-Simulation.git), for all concurrent matters
where several code snippets were used for the terminal user interface

* As well as material as taught from the CppND from [udacity](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213)

* Other single references are commented inside the code


## README (All Rubric Points REQUIRED) 

1. A `README` with instructions is included with the project
2. The `README` indicates which project is chosen
3. The `README` includes information about each rubric point addressed

## Compiling and Testing (All Rubric Points REQUIRED)

1. Code is compiling and running

## Loops, Functions, I/O - non written rubrics were not used

1. The project demonstrates an understanding of C++ functions and control structures

## Object Oriented Programming

1. The project uses Object Oriented Programming techniques -> refer to all code in `src/` folder
2. Classes use appropriate access specifiers for class members
3. Classes abstract implementation details from their interfaces
4. Classes encapsulate behavior -> refer to all private methods from code classes

## Memory Management

1. The project makes use of references in function declarations -> refer as an example to `Agent.cpp::12` (agent class constructor)
2. The project uses destructors appropriately -> refer to `WaitingArea.cpp::163`
3. The project uses scope / Resource Acquisition Is Initialization (RAII) where appropriate -> refer to `shared_ptr` throughout the code
4. The project uses move semantics to move data, instead of copying it, where possible -> refer to `WaitingArea.cpp::24` as an example
5. The project uses smart pointers instead of raw pointers

## Concurrency

1. The project uses multithreading -> refer to `WaitingArea.cpp::118`
2. A promise and future is used in the project -> refer to `WaitingArea.cpp::81`
3. A mutex or lock is used in the project -> refer to `WaitingArea.cpp::14`

