#define _POSIX_C_SOURCE 200809L
#include <ncurses.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
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
    int id;                  // Unique identifier
    Position pos;            // Floating-point position
    char character;          // Display character
    int color_pair;          // ncurses color pair index
    EntityType type;         // Type of entity
    bool passable;           // Can other entities move through this?
    bool active;             // Is this entity active (for pooling)
    char *name;              // Optional name for debugging
} Entity;

// Entity management
#define MAX_ENTITIES 1000
Entity entities[MAX_ENTITIES];
int next_entity_id = 0;
int entity_count = 0;

// Color pair definitions
#define COLOR_PAIR_WHITE_BLACK   1
#define COLOR_PAIR_RED_BLACK     2
#define COLOR_PAIR_GREEN_BLACK   3
#define COLOR_PAIR_YELLOW_BLACK  4
#define COLOR_PAIR_BLUE_BLACK    5
#define COLOR_PAIR_MAGENTA_BLACK 6
#define COLOR_PAIR_CYAN_BLACK    7
#define COLOR_PAIR_WHITE_RED     8
#define COLOR_PAIR_YELLOW_BLUE   9
#define COLOR_PAIR_BLACK_WHITE   10

// ============================================================================
// Initialization
// ============================================================================

void init_colors() {
    if (has_colors()) {
        start_color();

        // Basic color pairs
        init_pair(COLOR_PAIR_WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_RED_BLACK, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_PAIR_GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PAIR_BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_PAIR_MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_PAIR_CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);

        // Special combinations
        init_pair(COLOR_PAIR_WHITE_RED, COLOR_WHITE, COLOR_RED);
        init_pair(COLOR_PAIR_YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);
        init_pair(COLOR_PAIR_BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);
    }
}

void init_entities() {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].active = false;
        entities[i].name = NULL;
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

    // Find first inactive slot
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
    e->character = character;
    e->color_pair = color_pair;
    e->type = type;
    e->passable = passable;
    e->active = true;

    if (name) {
        e->name = strdup(name);
    } else {
        e->name = NULL;
    }

    entity_count++;
    return e;
}

void destroy_entity(Entity *entity) {
    if (entity && entity->active) {
        if (entity->name) {
            free(entity->name);
            entity->name = NULL;
        }
        entity->active = false;
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

    // Check if entity is passable
    return entity->passable;
}

// ============================================================================
// Coordinate Conversion
// ============================================================================

void world_to_screen(double world_x, double world_y, int *screen_x, int *screen_y) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Round to nearest grid position
    int grid_x = (int)round(world_x);
    int grid_y = (int)round(world_y);

    // Convert to screen coordinates (centered at 0,0)
    *screen_x = max_x / 2 + grid_x;
    *screen_y = max_y / 2 - grid_y;  // Invert Y axis so positive Y is up
}

// ============================================================================
// Rendering
// ============================================================================

void draw_map(Entity *player) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    clear();

    // Draw coordinate axes (optional, for visual reference)
    int center_x = max_x / 2;
    int center_y = max_y / 2;

    attron(COLOR_PAIR(COLOR_PAIR_BLUE_BLACK) | A_DIM);
    // Draw horizontal axis
    for (int x = 0; x < max_x; x++) {
        mvaddch(center_y, x, '-');
    }

    // Draw vertical axis
    for (int y = 0; y < max_y; y++) {
        mvaddch(y, center_x, '|');
    }

    // Draw origin marker
    mvaddch(center_y, center_x, '+');
    attroff(COLOR_PAIR(COLOR_PAIR_BLUE_BLACK) | A_DIM);

    // Draw all entities (except player, who we draw last)
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active || &entities[i] == player) continue;

        Entity *e = &entities[i];
        int screen_x, screen_y;
        world_to_screen(e->pos.x, e->pos.y, &screen_x, &screen_y);

        if (screen_x >= 0 && screen_x < max_x && screen_y >= 0 && screen_y < max_y) {
            attron(COLOR_PAIR(e->color_pair));
            mvaddch(screen_y, screen_x, e->character);
            attroff(COLOR_PAIR(e->color_pair));
        }
    }

    // Draw player last (so it's on top)
    if (player) {
        int screen_x, screen_y;
        world_to_screen(player->pos.x, player->pos.y, &screen_x, &screen_y);

        if (screen_x >= 0 && screen_x < max_x && screen_y >= 0 && screen_y < max_y) {
            attron(COLOR_PAIR(player->color_pair) | A_BOLD);
            mvaddch(screen_y, screen_x, player->character);
            attroff(COLOR_PAIR(player->color_pair) | A_BOLD);
        }
    }

    // Display info
    attron(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK));
    mvprintw(0, 0, "Position: (%.1f, %.1f) | Grid: (%d, %d) | Entities: %d | q to quit",
             player->pos.x, player->pos.y,
             (int)round(player->pos.x), (int)round(player->pos.y),
             entity_count);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK));

    // Show entity info at player's position
    Entity *at_player = find_entity_at(player->pos.x, player->pos.y);
    if (at_player && at_player != player) {
        attron(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
        mvprintw(1, 0, "Here: %s (%c) - %s",
                 at_player->name ? at_player->name : "Unknown",
                 at_player->character,
                 at_player->passable ? "passable" : "impassable");
        attroff(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
    }

    refresh();
}

// ============================================================================
// Sample World Creation
// ============================================================================

void create_sample_world() {
    // Create some scenery (trees)
    create_entity(-5.0, 3.0, 'T', COLOR_PAIR_GREEN_BLACK, ENTITY_SCENERY, true, "Pine Tree");
    create_entity(-3.0, 4.0, 'T', COLOR_PAIR_GREEN_BLACK, ENTITY_SCENERY, true, "Oak Tree");
    create_entity(4.0, -2.0, 'T', COLOR_PAIR_GREEN_BLACK, ENTITY_SCENERY, true, "Maple Tree");

    // Create some walls (impassable)
    for (int x = -2; x <= 2; x++) {
        create_entity((double)x, 5.0, '#', COLOR_PAIR_WHITE_BLACK, ENTITY_SCENERY, false, "Stone Wall");
    }
    for (int y = -3; y <= 3; y++) {
        create_entity(7.0, (double)y, '#', COLOR_PAIR_WHITE_BLACK, ENTITY_SCENERY, false, "Stone Wall");
    }

    // Create some items (passable)
    create_entity(-4.0, -1.0, '$', COLOR_PAIR_YELLOW_BLACK, ENTITY_ITEM, true, "Gold Coin");
    create_entity(2.0, 2.0, '!', COLOR_PAIR_MAGENTA_BLACK, ENTITY_ITEM, true, "Health Potion");
    create_entity(-1.0, -3.0, '/', COLOR_PAIR_CYAN_BLACK, ENTITY_ITEM, true, "Sword");

    // Create some mobs (impassable)
    create_entity(-6.0, -4.0, 'g', COLOR_PAIR_GREEN_BLACK, ENTITY_MOB, false, "Goblin");
    create_entity(5.0, 4.0, 'o', COLOR_PAIR_RED_BLACK, ENTITY_MOB, false, "Orc");

    // Create environmental hazards (passable but dangerous-looking)
    create_entity(0.0, -5.0, '^', COLOR_PAIR_WHITE_RED, ENTITY_ENVIRONMENTAL, true, "Fire");
    create_entity(3.0, -4.0, '~', COLOR_PAIR_GREEN_BLACK, ENTITY_ENVIRONMENTAL, true, "Acid Pool");

    // Create a pet (passable)
    create_entity(1.0, 1.0, 'd', COLOR_PAIR_YELLOW_BLACK, ENTITY_PET, true, "Dog");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    // Initialize ncurses
    initscr();
    cbreak();              // Disable line buffering
    noecho();              // Don't echo keypresses
    keypad(stdscr, TRUE);  // Enable arrow keys
    curs_set(0);           // Hide cursor

    init_colors();
    init_entities();

    // Create player
    Entity *player = create_entity(0.0, 0.0, '@', COLOR_PAIR_WHITE_BLACK, ENTITY_PLAYER, false, "Player");

    // Create sample world
    create_sample_world();

    // Main game loop
    int running = 1;
    draw_map(player);

    while (running) {
        int ch = getch();

        double new_x = player->pos.x;
        double new_y = player->pos.y;

        switch (ch) {
            case KEY_UP:
            case 'w':
            case 'W':
                new_y += 1.0;
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                new_y -= 1.0;
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                new_x -= 1.0;
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                new_x += 1.0;
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
        }

        // Check if new position is passable
        if (new_x != player->pos.x || new_y != player->pos.y) {
            if (is_position_passable(new_x, new_y, player)) {
                player->pos.x = new_x;
                player->pos.y = new_y;
            }
        }

        draw_map(player);
    }

    // Clean up
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].active) {
            destroy_entity(&entities[i]);
        }
    }

    endwin();

    return 0;
}
