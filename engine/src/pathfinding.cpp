#include "pathfinding.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace {

constexpr double DIAGONAL_COST = 1.41421356237;

// Estimated distance when you can move in 8 directions
double octileDistance(Point a, Point b) {
    int dx = std::abs(a.x - b.x);
    int dy = std::abs(a.y - b.y);
    return (dx + dy) + (DIAGONAL_COST - 2.0) * std::min(dx, dy);
}

struct Node {
    double f;  // cost so far + estimated cost left
    int index;
    bool operator>(const Node& other) const { return f > other.f; }
};

const Point DIRECTIONS[8] = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
};

}  // namespace

std::optional<PathResult> findPath(const Grid& grid, Point start, Point goal) {
    if (!grid.isWalkable(start) || !grid.isWalkable(goal)) return std::nullopt;

    const int width = grid.width();
    const int total = width * grid.height();
    auto toIndex = [width](Point p) { return p.y * width + p.x; };
    auto toPoint = [width](int i) { return Point{i % width, i / width}; };

    std::vector<double> costSoFar(total, std::numeric_limits<double>::infinity());
    std::vector<int> cameFrom(total, -1);
    std::vector<bool> done(total, false);
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    const int startIndex = toIndex(start);
    const int goalIndex = toIndex(goal);
    costSoFar[startIndex] = 0.0;
    open.push({octileDistance(start, goal), startIndex});

    int explored = 0;
    while (!open.empty()) {
        Node current = open.top();
        open.pop();
        if (done[current.index]) continue;
        done[current.index] = true;
        ++explored;

        if (current.index == goalIndex) {
            PathResult result;
            result.cost = costSoFar[goalIndex];
            result.nodesExplored = explored;
            for (int i = goalIndex; i != -1; i = cameFrom[i]) {
                result.path.push_back(toPoint(i));
            }
            std::reverse(result.path.begin(), result.path.end());
            return result;
        }

        Point p = toPoint(current.index);
        for (Point d : DIRECTIONS) {
            Point next{p.x + d.x, p.y + d.y};
            if (!grid.isWalkable(next)) continue;

            bool diagonal = d.x != 0 && d.y != 0;
            // No cutting corners around obstacles
            if (diagonal && (!grid.isWalkable({p.x + d.x, p.y}) || !grid.isWalkable({p.x, p.y + d.y}))) {
                continue;
            }

            int nextIndex = toIndex(next);
            if (done[nextIndex]) continue;

            double newCost = costSoFar[current.index] + (diagonal ? DIAGONAL_COST : 1.0);
            if (newCost < costSoFar[nextIndex]) {
                costSoFar[nextIndex] = newCost;
                cameFrom[nextIndex] = current.index;
                open.push({newCost + octileDistance(next, goal), nextIndex});
            }
        }
    }

    return std::nullopt;
}