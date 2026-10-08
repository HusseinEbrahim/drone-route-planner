cmake_minimum_required(VERSION 3.20)
project(drone_route_planner CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(planner_core
    src/grid.cpp
    src/pathfinding.cpp
    src/planner.cpp
)
target_include_directories(planner_core PUBLIC src)

add_executable(planner src/main.cpp)
target_link_libraries(planner PRIVATE planner_core)
