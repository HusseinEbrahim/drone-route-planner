#include "planner.h"
#include <algorithm>
#include <limits>
#include "pathfinding.h"

namespace {

constexpr double INF = std::numeric_limits<double>::infinity();

// Index 0 is home, indices 1..n are the stops
struct DistanceTable {
    std::vector<std::vector<double>> dist;
    std::vector<std::vector<std::vector<Point>>> paths;
};

DistanceTable buildTable(const Grid& grid, const std::vector<Point>& points) {
    const int n = static_cast<int>(points.size());
    DistanceTable table;
    table.dist.assign(n, std::vector<double>(n, INF));
    table.paths.assign(n, std::vector<std::vector<Point>>(n));

    for (int i = 0; i < n; ++i) {
        table.dist[i][i] = 0.0;
        table.paths[i][i] = {points[i]};
        for (int j = i + 1; j < n; ++j) {
            auto result = findPath(grid, points[i], points[j]);
            if (!result) continue;
            table.dist[i][j] = table.dist[j][i] = result->cost;
            table.paths[i][j] = result->path;
            table.paths[j][i] = std::vector<Point>(result->path.rbegin(), result->path.rend());
        }
    }
    return table;
}

double tourLength(const std::vector<int>& order, const DistanceTable& table) {
    double total = 0.0;
    int previous = 0;
    for (int stop : order) {
        total += table.dist[previous][stop];
        previous = stop;
    }
    return total + table.dist[previous][0];
}

std::vector<int> nearestNeighbourOrder(std::vector<int> remaining, const DistanceTable& table) {
    std::vector<int> order;
    int current = 0;
    while (!remaining.empty()) {
        auto closest = std::min_element(remaining.begin(), remaining.end(), [&](int a, int b) {
            return table.dist[current][a] < table.dist[current][b];
        });
        current = *closest;
        order.push_back(current);
        remaining.erase(closest);
    }
    return order;
}

void improveWithTwoOpt(std::vector<int>& order, const DistanceTable& table) {
    double best = tourLength(order, table);
    bool improved = true;
    while (improved) {
        improved = false;
        for (size_t i = 0; i + 1 < order.size(); ++i) {
            for (size_t j = i + 1; j < order.size(); ++j) {
                std::reverse(order.begin() + i, order.begin() + j + 1);
                double length = tourLength(order, table);
                if (length + 1e-9 < best) {
                    best = length;
                    improved = true;
                } else {
                    std::reverse(order.begin() + i, order.begin() + j + 1);  // undo
                }
            }
        }
    }
}

std::vector<std::vector<int>> splitByBattery(const std::vector<int>& order, const DistanceTable& table,
                                             double batteryRange) {
    std::vector<std::vector<int>> trips;
    std::vector<int> current;
    double used = 0.0;
    int last = 0;

    for (int stop : order) {
        double neededWithStop = used + table.dist[last][stop] + table.dist[stop][0];
        if (!current.empty() && neededWithStop > batteryRange) {
            trips.push_back(current);  // fly home and recharge
            current.clear();
            used = 0.0;
            last = 0;
        }
        current.push_back(stop);
        used += table.dist[last][stop];
        last = stop;
    }
    if (!current.empty()) trips.push_back(current);
    return trips;
}

}  // namespace

MissionPlan planMission(const Grid& grid, Point home, const std::vector<Point>& stops, double batteryRange) {
    std::vector<Point> points{home};
    points.insert(points.end(), stops.begin(), stops.end());
    DistanceTable table = buildTable(grid, points);

    MissionPlan plan;
    std::vector<int> reachable;
    for (int i = 1; i < static_cast<int>(points.size()); ++i) {
        // Must be able to reach the stop and get back on a full battery
        if (2 * table.dist[0][i] > batteryRange) plan.unreachableStops.push_back(i - 1);
        else reachable.push_back(i);
    }

    std::vector<int> order = nearestNeighbourOrder(reachable, table);
    improveWithTwoOpt(order, table);

    for (const auto& tripStops : splitByBattery(order, table, batteryRange)) {
        Trip trip;
        trip.path.push_back(home);
        int previous = 0;

        auto addLeg = [&](int from, int to) {
            const auto& leg = table.paths[from][to];
            trip.path.insert(trip.path.end(), leg.begin() + 1, leg.end());
            trip.distance += table.dist[from][to];
        };

        for (int stop : tripStops) {
            addLeg(previous, stop);
            trip.stopOrder.push_back(stop - 1);
            previous = stop;
        }
        addLeg(previous, 0);

        plan.totalDistance += trip.distance;
        plan.trips.push_back(std::move(trip));
    }
    return plan;
}