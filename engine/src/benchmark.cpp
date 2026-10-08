#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include "grid.h"
#include "pathfinding.h"
#include "planner.h"

using Clock = std::chrono::steady_clock;

Grid randomGrid(int size, double density, std::mt19937& rng) {
    Grid grid(size, size);
    std::bernoulli_distribution blocked(density);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (blocked(rng)) grid.set({x, y}, Cell::Obstacle);
        }
    }
    grid.set({0, 0}, Cell::Free);
    grid.set({size - 1, size - 1}, Cell::Free);
    return grid;
}

double millisecondsSince(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

int main() {
    std::mt19937 rng(42);  // fixed seed, so results are repeatable
    const int size = 100;

    // 1. Single A* search, corner to corner on a 100x100 map with 25% obstacles
    const int searches = 200;
    double searchMs = 0.0;
    int found = 0;
    for (int i = 0; i < searches; ++i) {
        Grid grid = randomGrid(size, 0.25, rng);
        auto start = Clock::now();
        auto result = findPath(grid, {0, 0}, {size - 1, size - 1});
        searchMs += millisecondsSince(start);
        if (result) ++found;
    }

    // 2. Full mission: 15 delivery stops on a 100x100 map with 20% obstacles
    const int missions = 50;
    double missionMs = 0.0;
    std::uniform_int_distribution<int> coord(0, size - 1);
    for (int i = 0; i < missions; ++i) {
        Grid grid = randomGrid(size, 0.2, rng);
        std::vector<Point> stops;
        while (stops.size() < 15) {
            Point p{coord(rng), coord(rng)};
            if (grid.isWalkable(p)) stops.push_back(p);
        }
        auto start = Clock::now();
        planMission(grid, {0, 0}, stops, 400.0);
        missionMs += millisecondsSince(start);
    }

    std::cout << "A* search (100x100, 25% obstacles):  " << searchMs / searches << " ms average ("
              << found << "/" << searches << " solvable)\n";
    std::cout << "Full mission (15 stops, 100x100):     " << missionMs / missions << " ms average\n";
    return 0;
}