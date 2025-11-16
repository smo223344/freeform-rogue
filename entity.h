#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <vector>

// Forward declaration
class MOB;

// ============================================================================
// Entity System
// ============================================================================

enum class EntityType {
    SCENERY,
    ITEM,
    MOB,
    PET,
    PROJECTILE,
    ENVIRONMENTAL,
    PLAYER
};

struct Position {
    double x;
    double y;

    Position() : x(0.0), y(0.0) {}
    Position(double x_, double y_) : x(x_), y(y_) {}
};

class EntityDefinition {
public:
    int id;                      // Unique identifier for this definition
    char character;              // Display character
    int color_pair;              // ncurses color pair index
    EntityType type;             // Type of entity
    bool passable;               // Can other entities move through this?
    bool active;                 // Is this definition active (for pooling)
    std::string name;            // Name for debugging/display

    EntityDefinition();
    void reset();
};

class Entity {
public:
    int id;                      // Unique identifier
    Position pos;                // Floating-point position
    int definition_id;           // Index into entity_definitions array
    bool active;                 // Is this entity instance active?
    MOB* mob;                    // Optional MOB (for entities with HP/AI)

    Entity();
    void reset();
};

// Entity management constants
constexpr int MAX_ENTITIES = 1000;
constexpr int MAX_ENTITY_DEFINITIONS = 100;

// Global entity storage (defined in entity.cpp)
extern std::vector<Entity> entities;
extern EntityDefinition entity_definitions[MAX_ENTITY_DEFINITIONS];
extern int next_entity_id;
extern int next_entity_definition_id;
extern int entity_definition_count;

// ============================================================================
// Function Declarations
// ============================================================================

// Initialization
void init_entities();

// Entity Definition Management
EntityDefinition* create_entity_definition(char character, int color_pair,
                                           EntityType type, bool passable, const std::string& name);
EntityDefinition* get_entity_definition(int definition_id);
void destroy_entity_definition(EntityDefinition* def);

// Entity Management
Entity* create_entity(double x, double y, char character, int color_pair,
                      EntityType type, bool passable, const std::string& name);
void destroy_entity(Entity* entity);
Entity* find_entity_at(double x, double y);
bool is_position_passable(double x, double y, Entity* ignore);

#endif // ENTITY_H
