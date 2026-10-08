#pragma once
#include <vector>
#include "grid.h"

struct Trip {
    std::vector<int> stopOrder;  // indices into the original stops list
    std::vector<Point> path;     // full flight path: home -> stops -> home
    double distance = 0.0;
};

struct MissionPlan {
    std::vector<Trip> trips;
    std::vector<int> unreachableStops;  // blocked, or too far for the battery
    double totalDistance = 0.0;
};

MissionPlan planMission(const Grid& grid, Point home, const std::vector<Point>& stops, double batteryRange);