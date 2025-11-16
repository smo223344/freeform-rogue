#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include "entity.h"

// Forward declaration
class MOB;

// ============================================================================
// Path Class - Represents a path from one position to another
// ============================================================================

class Path {
public:
    // Construct an invalid path (no path found)
    Path();

    // Construct a valid path from a list of positions
    Path(const std::vector<Position>& waypoints);

    // Check if the path is valid (a path was found)
    bool is_valid() const;

    // Get the total distance of the path
    double get_distance() const;

    // Get the direction of the next move (returns normalized direction vector)
    // Returns Position(0, 0) if no valid next move
    Position get_next_direction() const;

    // Get the next position to move to
    // Returns the current position if path is invalid or empty
    Position get_next_position() const;

    // Get the full path as a vector of positions
    const std::vector<Position>& get_waypoints() const;

    // Advance along the path (removes the first waypoint)
    void advance();

    // Static factory for creating a "no path" result
    static Path no_path();

private:
    std::vector<Position> waypoints_;
    bool valid_;
    double distance_;

    void calculate_distance();
};

// ============================================================================
// Pathfinding - A* pathfinding system
// ============================================================================

class Pathfinding {
public:
    // Find a path from start MOB to target MOB
    // If ignore_mobs is true, only terrain blocking is considered
    // If ignore_mobs is false, other MOBs are treated as obstacles
    static Path find_path(MOB* start, MOB* target, bool ignore_mobs = false);

    // Find a path from start position to target position
    static Path find_path(Position start, Position target, bool ignore_mobs = false);

private:
    // Internal A* implementation
    static Path a_star(Position start, Position target, bool ignore_mobs);

    // Heuristic function (Manhattan distance)
    static double heuristic(Position a, Position b);

    // Check if a position is walkable
    static bool is_walkable(Position pos, bool ignore_mobs);

    // Get neighbors of a position (4-directional: up, down, left, right)
    static std::vector<Position> get_neighbors(Position pos);

    // Reconstruct path from came_from map
    static std::vector<Position> reconstruct_path(
        const std::vector<std::vector<Position>>& came_from,
        Position start, Position goal);
};

#endif // PATHFINDING_H
