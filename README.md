# Agent-based model 

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
