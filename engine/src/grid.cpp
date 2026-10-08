#include "grid.h"
#include <stdexcept>

Grid::Grid(int width, int height)
    : width_(width), height_(height), cells_(width * height, Cell::Free) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
}

Grid Grid::fromStrings(const std::vector<std::string>& rows) {
    if (rows.empty()) throw std::invalid_argument("Map is empty");

    Grid grid(static_cast<int>(rows[0].size()), static_cast<int>(rows.size()));
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            char c = rows[y][x];
            if (c == '#') grid.set({x, y}, Cell::Obstacle);
            else if (c == 'N') grid.set({x, y}, Cell::NoFly);
        }
    }
    return grid;
}

bool Grid::inBounds(Point p) const {
    return p.x >= 0 && p.x < width_ && p.y >= 0 && p.y < height_;
}

bool Grid::isWalkable(Point p) const {
    return inBounds(p) && cells_[index(p)] == Cell::Free;
}

Cell Grid::get(Point p) const {
    return cells_[index(p)];
}

void Grid::set(Point p, Cell cell) {
    cells_[index(p)] = cell;
}