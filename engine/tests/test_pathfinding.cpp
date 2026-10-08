#include <cmath>
#include <cstdlib>
#include <gtest/gtest.h>
#include "grid.h"
#include "pathfinding.h"

TEST(Pathfinding, StraightLineOnEmptyGrid) {
    Grid grid(10, 10);
    auto result = findPath(grid, {0, 0}, {4, 0});
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->cost, 4.0);
    EXPECT_EQ(result->path.size(), 5u);
}

TEST(Pathfinding, DiagonalCostsSqrtTwo) {
    Grid grid(10, 10);
    auto result = findPath(grid, {0, 0}, {3, 3});
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->cost, 3 * std::sqrt(2.0), 1e-6);
}

TEST(Pathfinding, AvoidsObstaclesAndNoFlyZones) {
    Grid grid = Grid::fromStrings({
        "....#....",
        "....#....",
        "....N....",
        "....N....",
        ".........",
    });
    auto result = findPath(grid, {0, 0}, {8, 0});
    ASSERT_TRUE(result.has_value());
    for (const Point& p : result->path) {
        EXPECT_TRUE(grid.isWalkable(p));
    }
}

TEST(Pathfinding, PathMovesOneSquareAtATime) {
    Grid grid = Grid::fromStrings({
        "..#.....",
        "..#..#..",
        "..#..#..",
        ".....#..",
    });
    auto result = findPath(grid, {0, 0}, {7, 0});
    ASSERT_TRUE(result.has_value());
    for (size_t i = 1; i < result->path.size(); ++i) {
        EXPECT_LE(std::abs(result->path[i].x - result->path[i - 1].x), 1);
        EXPECT_LE(std::abs(result->path[i].y - result->path[i - 1].y), 1);
    }
}

TEST(Pathfinding, DoesNotCutCorners) {
    Grid grid = Grid::fromStrings({
        ".#",
        "..",
    });
    auto result = findPath(grid, {0, 0}, {1, 1});
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->cost, 2.0);  // must go around, not diagonally past the corner
}

TEST(Pathfinding, ReturnsNothingWhenGoalIsWalledOff) {
    Grid grid = Grid::fromStrings({
        ".....",
        "..###",
        "..#..",
        "..###",
    });
    EXPECT_FALSE(findPath(grid, {0, 0}, {3, 2}).has_value());
}

TEST(Pathfinding, ReturnsNothingWhenStartIsBlocked) {
    Grid grid = Grid::fromStrings({"#...."});
    EXPECT_FALSE(findPath(grid, {0, 0}, {4, 0}).has_value());
}