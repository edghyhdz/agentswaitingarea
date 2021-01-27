cmake_minimum_required(VERSION 3.11.3)

# set(CMAKE_CXX_STANDARD 17)
project(Agent_based_waiting_area)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -lcurses")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})
include_directories(include)

# Find all executables
file(GLOB project_SRCS src/*.cpp) #src/*.h

# Add project executable
add_executable(agents_display ${project_SRCS})

set_property(TARGET agents_display PROPERTY CXX_STANDARD 17)
target_link_libraries(agents_display ${CURSES_LIBRARIES})
