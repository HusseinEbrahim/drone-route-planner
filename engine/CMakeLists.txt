cmake_minimum_required(VERSION 3.20)
project(drone_route_planner CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(httplib json)

add_library(planner_core
    src/grid.cpp
    src/pathfinding.cpp
    src/planner.cpp
)
target_include_directories(planner_core PUBLIC src)

# Command-line demo
add_executable(planner src/main.cpp)
target_link_libraries(planner PRIVATE planner_core)

# HTTP API server
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE planner_core httplib::httplib nlohmann_json::nlohmann_json)
if (WIN32)
    target_compile_definitions(server PRIVATE _WIN32_WINNT=0x0A00)
endif()