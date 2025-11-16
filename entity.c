#define _POSIX_C_SOURCE 200809L
#include "entity.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// Entity Storage
// ============================================================================

Entity entities[MAX_ENTITIES];
EntityDefinition entity_definitions[MAX_ENTITY_DEFINITIONS];
int next_entity_id = 0;
int entity_count = 0;
int next_entity_definition_id = 0;
int entity_definition_count = 0;

// ============================================================================
// Initialization
// ============================================================================

void init_entities(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].active = false;
        entities[i].id = -1;
        entities[i].definition_id = -1;
    }
    for (int i = 0; i < MAX_ENTITY_DEFINITIONS; i++) {
        entity_definitions[i].active = false;
        entity_definitions[i].id = -1;
        entity_definitions[i].name = NULL;
    }
}

// ============================================================================
// Entity Definition Management
// ============================================================================

EntityDefinition* create_entity_definition(char character, int color_pair,
                                           EntityType type, bool passable, const char *name) {
    if (entity_definition_count >= MAX_ENTITY_DEFINITIONS) {
        return NULL;  // Definition pool full
    }

    // Find first inactive slot
    int slot = -1;
    for (int i = 0; i < MAX_ENTITY_DEFINITIONS; i++) {
        if (!entity_definitions[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) return NULL;

    EntityDefinition *def = &entity_definitions[slot];
    def->id = next_entity_definition_id++;
    def->character = character;
    def->color_pair = color_pair;
    def->type = type;
    def->passable = passable;
    def->active = true;

    if (name) {
        def->name = strdup(name);
    } else {
        def->name = NULL;
    }

    entity_definition_count++;
    return def;
}

EntityDefinition* get_entity_definition(int definition_id) {
    if (definition_id < 0 || definition_id >= MAX_ENTITY_DEFINITIONS) {
        return NULL;
    }
    if (!entity_definitions[definition_id].active) {
        return NULL;
    }
    return &entity_definitions[definition_id];
}

void destroy_entity_definition(EntityDefinition *def) {
    if (def && def->active) {
        if (def->name) {
            free(def->name);
            def->name = NULL;
        }
        def->active = false;
        entity_definition_count--;
    }
}

// ============================================================================
// Entity Management
// ============================================================================

Entity* create_entity(double x, double y, char character, int color_pair,
                      EntityType type, bool passable, const char *name) {
    if (entity_count >= MAX_ENTITIES) {
        return NULL;  // Entity pool full
    }

    // Create entity definition
    EntityDefinition *def = create_entity_definition(character, color_pair, type, passable, name);
    if (!def) {
        return NULL;  // Failed to create definition
    }

    // Find first inactive slot for entity
    int slot = -1;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) return NULL;

    Entity *e = &entities[slot];
    e->id = next_entity_id++;
    e->pos.x = x;
    e->pos.y = y;
    e->definition_id = def->id;
    e->active = true;

    entity_count++;
    return e;
}

void destroy_entity(Entity *entity) {
    if (entity && entity->active) {
        entity->active = false;
        entity->definition_id = -1;
        entity_count--;
    }
}

Entity* find_entity_at(double x, double y) {
    int grid_x = (int)round(x);
    int grid_y = (int)round(y);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active) {
            int ex = (int)round(entities[i].pos.x);
            int ey = (int)round(entities[i].pos.y);
            if (ex == grid_x && ey == grid_y) {
                return &entities[i];
            }
        }
    }
    return NULL;
}

bool is_position_passable(double x, double y, Entity *ignore) {
    Entity *entity = find_entity_at(x, y);

    // No entity at position, it's passable
    if (!entity) return true;

    // Ignore specific entity (e.g., don't collide with self)
    if (entity == ignore) return true;

    // Get entity definition and check if passable
    EntityDefinition *def = get_entity_definition(entity->definition_id);
    if (!def) return true;  // If definition missing, assume passable

    return def->passable;
}
