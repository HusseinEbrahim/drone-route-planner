#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "grid.h"
#include "planner.h"

using json = nlohmann::json;

namespace {

constexpr int MAX_SIZE = 100;
constexpr int MAX_STOPS = 15;

Point parsePoint(const json& value) {
    if (!value.is_array() || value.size() != 2) {
        throw std::invalid_argument("Points must be [x, y]");
    }
    return {value.at(0).get<int>(), value.at(1).get<int>()};
}

json pointToJson(Point p) {
    return json::array({p.x, p.y});
}

void sendError(httplib::Response& res, int status, const std::string& message) {
    res.status = status;
    res.set_content(json{{"error", message}}.dump(), "application/json");
}

void addCells(Grid& grid, const json& points, Cell cell) {
    for (const auto& value : points) {
        Point p = parsePoint(value);
        if (!grid.inBounds(p)) throw std::invalid_argument("Cell outside the map");
        grid.set(p, cell);
    }
}

}  // namespace

int main() {
    httplib::Server server;

    // Allow the React frontend (on a different address) to call this API
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"},
    });
    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    server.Post("/plan", [](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            int width = body.at("width").get<int>();
            int height = body.at("height").get<int>();
            if (width < 2 || height < 2 || width > MAX_SIZE || height > MAX_SIZE) {
                return sendError(res, 400, "Map size must be between 2 and 100");
            }

            Grid grid(width, height);
            addCells(grid, body.value("obstacles", json::array()), Cell::Obstacle);
            addCells(grid, body.value("noFly", json::array()), Cell::NoFly);

            Point home = parsePoint(body.at("home"));
            if (!grid.isWalkable(home)) {
                return sendError(res, 400, "Home must be on an open square");
            }

            std::vector<Point> stops;
            for (const auto& value : body.at("stops")) {
                Point p = parsePoint(value);
                if (!grid.inBounds(p)) return sendError(res, 400, "Stop outside the map");
                stops.push_back(p);
            }
            if (stops.empty() || static_cast<int>(stops.size()) > MAX_STOPS) {
                return sendError(res, 400, "Add between 1 and 15 stops");
            }

            double batteryRange = body.at("batteryRange").get<double>();
            if (batteryRange <= 0) return sendError(res, 400, "Battery range must be positive");

            auto started = std::chrono::steady_clock::now();
            MissionPlan plan = planMission(grid, home, stops, batteryRange);
            double elapsedMs = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - started).count();

            json trips = json::array();
            for (const Trip& trip : plan.trips) {
                json path = json::array();
                for (Point p : trip.path) path.push_back(pointToJson(p));
                trips.push_back({
                    {"stops", trip.stopOrder},
                    {"path", path},
                    {"distance", trip.distance},
                });
            }

            json response = {
                {"trips", trips},
                {"unreachableStops", plan.unreachableStops},
                {"totalDistance", plan.totalDistance},
                {"computeTimeMs", elapsedMs},
            };
            res.set_content(response.dump(), "application/json");
        } catch (const json::exception& e) {
            sendError(res, 400, std::string("Invalid request: ") + e.what());
        } catch (const std::invalid_argument& e) {
            sendError(res, 400, e.what());
        }
    });

    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::atoi(portEnv) : 8080;
    std::cout << "Route planner API listening on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    return 0;
}