#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>

// ============================================================================
// Entity System
// ============================================================================

typedef enum {
    ENTITY_SCENERY,
    ENTITY_ITEM,
    ENTITY_MOB,
    ENTITY_PET,
    ENTITY_PROJECTILE,
    ENTITY_ENVIRONMENTAL,
    ENTITY_PLAYER
} EntityType;

typedef struct {
    double x;
    double y;
} Position;

typedef struct {
    int id;                  // Unique identifier for this definition
    char character;          // Display character
    int color_pair;          // ncurses color pair index
    EntityType type;         // Type of entity
    bool passable;           // Can other entities move through this?
    bool active;             // Is this definition active (for pooling)
    char *name;              // Optional name for debugging
} EntityDefinition;

typedef struct {
    int id;                  // Unique identifier
    Position pos;            // Floating-point position
    int definition_id;       // Index into entity_definitions array
    bool active;             // Is this entity instance active?
} Entity;

// Entity management constants
#define MAX_ENTITIES 1000
#define MAX_ENTITY_DEFINITIONS 100

// Global entity storage (defined in entity.c)
extern Entity entities[MAX_ENTITIES];
extern EntityDefinition entity_definitions[MAX_ENTITY_DEFINITIONS];
extern int next_entity_id;
extern int entity_count;
extern int next_entity_definition_id;
extern int entity_definition_count;

// ============================================================================
// Function Declarations
// ============================================================================

// Initialization
void init_entities(void);

// Entity Definition Management
EntityDefinition* create_entity_definition(char character, int color_pair,
                                           EntityType type, bool passable, const char *name);
EntityDefinition* get_entity_definition(int definition_id);
void destroy_entity_definition(EntityDefinition *def);

// Entity Management
Entity* create_entity(double x, double y, char character, int color_pair,
                      EntityType type, bool passable, const char *name);
void destroy_entity(Entity *entity);
Entity* find_entity_at(double x, double y);
bool is_position_passable(double x, double y, Entity *ignore);

#endif // ENTITY_H
