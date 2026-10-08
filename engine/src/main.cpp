#include <iostream>
#include <string>
#include <vector>
#include "grid.h"
#include "pathfinding.h"

int main() {
    std::vector<std::string> map = {
        "S...........",
        ".....#......",
        ".....#...NN.",
        ".....#...NN.",
        ".....#......",
        "........#...",
        "...NNN..#...",
        "........#..G",
    };

    Point start{}, goal{};
    for (int y = 0; y < static_cast<int>(map.size()); ++y) {
        for (int x = 0; x < static_cast<int>(map[y].size()); ++x) {
            if (map[y][x] == 'S') start = {x, y};
            if (map[y][x] == 'G') goal = {x, y};
        }
    }

    Grid grid = Grid::fromStrings(map);
    auto result = findPath(grid, start, goal);

    if (!result) {
        std::cout << "No path found\n";
        return 1;
    }

    for (const Point& p : result->path) {
        if (map[p.y][p.x] == '.') map[p.y][p.x] = '*';
    }
    for (const auto& row : map) std::cout << row << '\n';

    std::cout << "\nPath length: " << result->path.size() << " squares\n";
    std::cout << "Distance:    " << result->cost << '\n';
    std::cout << "Explored:    " << result->nodesExplored << " squares\n";
    return 0;
}