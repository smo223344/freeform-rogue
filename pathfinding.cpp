#include "pathfinding.h"
#include "mob.h"
#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <algorithm>

// ============================================================================
// Helper structures for A*
// ============================================================================

// Hash function for Position (for unordered_map/set)
struct PositionHash {
    std::size_t operator()(const Position& pos) const {
        // Convert to grid coordinates and hash
        int x = static_cast<int>(std::round(pos.x));
        int y = static_cast<int>(std::round(pos.y));
        return std::hash<int>()(x) ^ (std::hash<int>()(y) << 1);
    }
};

// Equality function for Position
struct PositionEqual {
    bool operator()(const Position& a, const Position& b) const {
        int ax = static_cast<int>(std::round(a.x));
        int ay = static_cast<int>(std::round(a.y));
        int bx = static_cast<int>(std::round(b.x));
        int by = static_cast<int>(std::round(b.y));
        return ax == bx && ay == by;
    }
};

// Node for A* priority queue
struct AStarNode {
    Position pos;
    double f_score;

    bool operator>(const AStarNode& other) const {
        return f_score > other.f_score;
    }
};

// ============================================================================
// Path Class Implementation
// ============================================================================

Path::Path() : valid_(false), distance_(0.0) {
}

Path::Path(const std::vector<Position>& waypoints)
    : waypoints_(waypoints), valid_(!waypoints.empty()), distance_(0.0) {
    if (valid_) {
        calculate_distance();
    }
}

bool Path::is_valid() const {
    return valid_ && !waypoints_.empty();
}

double Path::get_distance() const {
    return distance_;
}

Position Path::get_next_direction() const {
    if (!is_valid() || waypoints_.size() < 2) {
        return Position(0.0, 0.0);
    }

    // Direction from first waypoint to second waypoint
    Position current = waypoints_[0];
    Position next = waypoints_[1];

    double dx = next.x - current.x;
    double dy = next.y - current.y;

    // Normalize to unit direction (should already be unit for grid-based movement)
    double length = std::sqrt(dx * dx + dy * dy);
    if (length > 0.0001) {
        dx /= length;
        dy /= length;
    }

    return Position(dx, dy);
}

Position Path::get_next_position() const {
    if (!is_valid() || waypoints_.empty()) {
        return Position(0.0, 0.0);
    }

    // Return the next waypoint (skip the current position which is waypoints_[0])
    if (waypoints_.size() > 1) {
        return waypoints_[1];
    }

    return waypoints_[0];
}

const std::vector<Position>& Path::get_waypoints() const {
    return waypoints_;
}

void Path::advance() {
    if (!waypoints_.empty()) {
        waypoints_.erase(waypoints_.begin());
        if (waypoints_.empty()) {
            valid_ = false;
        }
        calculate_distance();
    }
}

Path Path::no_path() {
    return Path();
}

void Path::calculate_distance() {
    distance_ = 0.0;
    for (size_t i = 1; i < waypoints_.size(); i++) {
        double dx = waypoints_[i].x - waypoints_[i - 1].x;
        double dy = waypoints_[i].y - waypoints_[i - 1].y;
        distance_ += std::sqrt(dx * dx + dy * dy);
    }
}

// ============================================================================
// Pathfinding Implementation
// ============================================================================

Path Pathfinding::find_path(MOB* start, MOB* target, bool ignore_mobs) {
    if (!start || !start->entity || !target || !target->entity) {
        return Path::no_path();
    }

    return find_path(start->entity->pos, target->entity->pos, ignore_mobs);
}

Path Pathfinding::find_path(Position start, Position target, bool ignore_mobs) {
    return a_star(start, target, ignore_mobs);
}

Path Pathfinding::a_star(Position start, Position target, bool ignore_mobs) {
    // Convert to grid coordinates
    int start_x = static_cast<int>(std::round(start.x));
    int start_y = static_cast<int>(std::round(start.y));
    int goal_x = static_cast<int>(std::round(target.x));
    int goal_y = static_cast<int>(std::round(target.y));

    Position start_grid(start_x, start_y);
    Position goal_grid(goal_x, goal_y);

    // Check if start and goal are the same
    if (start_x == goal_x && start_y == goal_y) {
        return Path({start_grid});
    }

    // Check if goal is walkable
    if (!is_walkable(goal_grid, ignore_mobs)) {
        return Path::no_path();
    }

    // A* data structures
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> open_set;
    std::unordered_set<Position, PositionHash, PositionEqual> closed_set;
    std::unordered_map<Position, Position, PositionHash, PositionEqual> came_from;
    std::unordered_map<Position, double, PositionHash, PositionEqual> g_score;

    // Initialize
    g_score[start_grid] = 0.0;
    open_set.push({start_grid, heuristic(start_grid, goal_grid)});

    while (!open_set.empty()) {
        AStarNode current = open_set.top();
        open_set.pop();

        // Skip if already processed
        if (closed_set.count(current.pos) > 0) {
            continue;
        }

        // Mark as processed
        closed_set.insert(current.pos);

        // Check if we reached the goal
        int curr_x = static_cast<int>(std::round(current.pos.x));
        int curr_y = static_cast<int>(std::round(current.pos.y));
        if (curr_x == goal_x && curr_y == goal_y) {
            // Reconstruct path
            std::vector<Position> path;
            Position pos = current.pos;
            path.push_back(pos);

            while (came_from.count(pos) > 0) {
                pos = came_from[pos];
                path.push_back(pos);
            }

            std::reverse(path.begin(), path.end());
            return Path(path);
        }

        // Explore neighbors
        std::vector<Position> neighbors = get_neighbors(current.pos);
        for (const Position& neighbor : neighbors) {
            // Skip if already processed
            if (closed_set.count(neighbor) > 0) {
                continue;
            }

            // Skip if not walkable (unless it's the goal)
            int nb_x = static_cast<int>(std::round(neighbor.x));
            int nb_y = static_cast<int>(std::round(neighbor.y));
            bool is_goal = (nb_x == goal_x && nb_y == goal_y);
            if (!is_goal && !is_walkable(neighbor, ignore_mobs)) {
                continue;
            }

            // Calculate tentative g_score
            double tentative_g = g_score[current.pos] + 1.0;  // Grid distance is 1

            // Update if this is a better path
            if (g_score.count(neighbor) == 0 || tentative_g < g_score[neighbor]) {
                came_from[neighbor] = current.pos;
                g_score[neighbor] = tentative_g;
                double f_score = tentative_g + heuristic(neighbor, goal_grid);
                open_set.push({neighbor, f_score});
            }
        }
    }

    // No path found
    return Path::no_path();
}

double Pathfinding::heuristic(Position a, Position b) {
    // Manhattan distance (appropriate for 4-directional movement)
    double dx = std::abs(a.x - b.x);
    double dy = std::abs(a.y - b.y);
    return dx + dy;
}

bool Pathfinding::is_walkable(Position pos, bool ignore_mobs) {
    // Check terrain - use the is_position_passable function
    // We need to check if there's an impassable entity at this position
    Entity* entity_at = find_entity_at(pos.x, pos.y);

    if (!entity_at) {
        return true;  // Empty space is walkable
    }

    // Get entity definition to check passability
    EntityDefinition* def = get_entity_definition(entity_at->definition_id);
    if (!def) {
        return true;  // If no definition, assume passable
    }

    // If entity is impassable terrain, can't walk here
    if (!def->passable) {
        return false;
    }

    // If ignoring MOBs, we're done (passable terrain)
    if (ignore_mobs) {
        return true;
    }

    // If not ignoring MOBs, check if there's a MOB here
    // MOBs block movement (except we might be trying to path to a MOB's location)
    if (entity_at->mob) {
        return false;  // MOB blocks this position
    }

    return true;  // Passable and no MOB
}

std::vector<Position> Pathfinding::get_neighbors(Position pos) {
    std::vector<Position> neighbors;

    // 4-directional movement (up, down, left, right)
    neighbors.push_back(Position(pos.x + 1, pos.y));      // Right
    neighbors.push_back(Position(pos.x - 1, pos.y));      // Left
    neighbors.push_back(Position(pos.x, pos.y + 1));      // Up
    neighbors.push_back(Position(pos.x, pos.y - 1));      // Down

    return neighbors;
}
