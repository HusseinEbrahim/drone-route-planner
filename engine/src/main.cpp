#include <iostream>
#include <string>
#include <vector>
#include "grid.h"
#include "planner.h"

int main() {
    std::vector<std::string> map = {
        "H.......#...........",
        "........#.....2.....",
        "...1....#...........",
        "........#....NNNN...",
        "..............NNN..3",
        "....####............",
        "..........#.........",
        ".4........#....NNN..",
        "..........#.....5...",
        "....................",
    };
    const double batteryRange = 45.0;

    Point home{};
    std::vector<Point> stops;
    std::vector<char> labels;
    for (char label = '1'; label <= '9'; ++label) {
        for (int y = 0; y < static_cast<int>(map.size()); ++y) {
            for (int x = 0; x < static_cast<int>(map[y].size()); ++x) {
                if (map[y][x] == label) {
                    stops.push_back({x, y});
                    labels.push_back(label);
                }
                if (label == '1' && map[y][x] == 'H') home = {x, y};
            }
        }
    }

    Grid grid = Grid::fromStrings(map);
    MissionPlan plan = planMission(grid, home, stops, batteryRange);

    for (size_t t = 0; t < plan.trips.size(); ++t) {
        const Trip& trip = plan.trips[t];
        std::cout << "Trip " << t + 1 << ": H";
        for (int s : trip.stopOrder) std::cout << " -> " << labels[s];
        std::cout << " -> H  (distance " << trip.distance << ")\n";

        for (const Point& p : trip.path) {
            if (map[p.y][p.x] == '.') map[p.y][p.x] = '*';
        }
    }

    std::cout << "\nTotal distance: " << plan.totalDistance << '\n';
    if (!plan.unreachableStops.empty()) {
        std::cout << "Unreachable stops:";
        for (int s : plan.unreachableStops) std::cout << ' ' << labels[s];
        std::cout << '\n';
    }

    std::cout << '\n';
    for (const auto& row : map) std::cout << row << '\n';
    return 0;
}