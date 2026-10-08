#pragma once
#include <optional>
#include <vector>
#include "grid.h"

struct PathResult {
    std::vector<Point> path;  // includes start and goal
    double cost = 0.0;
    int nodesExplored = 0;
};

std::optional<PathResult> findPath(const Grid& grid, Point start, Point goal);