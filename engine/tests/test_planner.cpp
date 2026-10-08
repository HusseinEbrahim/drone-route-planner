#include <algorithm>
#include <gtest/gtest.h>
#include "grid.h"
#include "planner.h"

TEST(Planner, VisitsEveryStopExactlyOnceWithBigBattery) {
    Grid grid(20, 20);
    std::vector<Point> stops = {{5, 5}, {15, 2}, {10, 18}, {2, 12}};
    MissionPlan plan = planMission(grid, {0, 0}, stops, 1000.0);

    ASSERT_EQ(plan.trips.size(), 1u);
    std::vector<int> visited = plan.trips[0].stopOrder;
    std::sort(visited.begin(), visited.end());
    EXPECT_EQ(visited, (std::vector<int>{0, 1, 2, 3}));
    EXPECT_TRUE(plan.unreachableStops.empty());
}

TEST(Planner, FindsOptimalOrderOnAStraightLine) {
    Grid grid(10, 3);
    std::vector<Point> stops = {{6, 0}, {2, 0}, {4, 0}};
    MissionPlan plan = planMission(grid, {0, 0}, stops, 1000.0);

    EXPECT_DOUBLE_EQ(plan.totalDistance, 12.0);  // out to x=6 and back
}

TEST(Planner, SplitsIntoTripsWithinBatteryRange) {
    Grid grid(30, 30);
    std::vector<Point> stops = {{20, 0}, {0, 20}, {20, 20}, {25, 5}};
    const double battery = 60.0;
    MissionPlan plan = planMission(grid, {0, 0}, stops, battery);

    EXPECT_GT(plan.trips.size(), 1u);
    for (const Trip& trip : plan.trips) {
        EXPECT_LE(trip.distance, battery + 1e-9);
    }
}

TEST(Planner, EveryTripStartsAndEndsAtHome) {
    Grid grid(30, 30);
    Point home{3, 3};
    std::vector<Point> stops = {{20, 0}, {0, 20}, {20, 20}};
    MissionPlan plan = planMission(grid, home, stops, 60.0);

    for (const Trip& trip : plan.trips) {
        EXPECT_EQ(trip.path.front(), home);
        EXPECT_EQ(trip.path.back(), home);
    }
}

TEST(Planner, MarksStopsBeyondBatteryAsUnreachable) {
    Grid grid(50, 5);
    std::vector<Point> stops = {{5, 0}, {45, 0}};
    MissionPlan plan = planMission(grid, {0, 0}, stops, 20.0);

    EXPECT_EQ(plan.unreachableStops, (std::vector<int>{1}));
    ASSERT_EQ(plan.trips.size(), 1u);
    EXPECT_EQ(plan.trips[0].stopOrder, (std::vector<int>{0}));
}

TEST(Planner, MarksWalledOffStopsAsUnreachable) {
    Grid grid = Grid::fromStrings({
        ".......",
        "....###",
        "....#..",
        "....###",
    });
    std::vector<Point> stops = {{2, 2}, {5, 2}};
    MissionPlan plan = planMission(grid, {0, 0}, stops, 1000.0);

    EXPECT_EQ(plan.unreachableStops, (std::vector<int>{1}));
}