   cmake_minimum_required(VERSION 3.20)
   project(drone_route_planner CXX)

   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   add_executable(planner src/main.cpp)