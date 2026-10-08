#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Point {
    int x = 0;
    int y = 0;
    bool operator==(const Point& other) const = default;
};

enum class Cell : uint8_t { Free, Obstacle, NoFly };

class Grid {
public:
    Grid(int width, int height);
    static Grid fromStrings(const std::vector<std::string>& rows);

    int width() const { return width_; }
    int height() const { return height_; }

    bool inBounds(Point p) const;
    bool isWalkable(Point p) const;
    Cell get(Point p) const;
    void set(Point p, Cell cell);

private:
    int width_;
    int height_;
    std::vector<Cell> cells_;

    int index(Point p) const { return p.y * width_ + p.x; }
};