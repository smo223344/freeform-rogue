#include <ncurses.h>
#include <cmath>
#include "entity.h"

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

// ============================================================================
// Coordinate Conversion
// ============================================================================

void world_to_screen(double world_x, double world_y, int* screen_x, int* screen_y) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Round to nearest grid position
    int grid_x = static_cast<int>(std::round(world_x));
    int grid_y = static_cast<int>(std::round(world_y));

    // Convert to screen coordinates (centered at 0,0)
    *screen_x = max_x / 2 + grid_x;
    *screen_y = max_y / 2 - grid_y;  // Invert Y axis so positive Y is up
}

// ============================================================================
// Rendering
// ============================================================================

void draw_map(Entity* player) {
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

        Entity* e = &entities[i];
        EntityDefinition* def = get_entity_definition(e->definition_id);
        if (!def) continue;  // Skip if definition missing

        int screen_x, screen_y;
        world_to_screen(e->pos.x, e->pos.y, &screen_x, &screen_y);

        if (screen_x >= 0 && screen_x < max_x && screen_y >= 0 && screen_y < max_y) {
            attron(COLOR_PAIR(def->color_pair));
            mvaddch(screen_y, screen_x, def->character);
            attroff(COLOR_PAIR(def->color_pair));
        }
    }

    // Draw player last (so it's on top)
    if (player) {
        EntityDefinition* player_def = get_entity_definition(player->definition_id);
        if (player_def) {
            int screen_x, screen_y;
            world_to_screen(player->pos.x, player->pos.y, &screen_x, &screen_y);

            if (screen_x >= 0 && screen_x < max_x && screen_y >= 0 && screen_y < max_y) {
                attron(COLOR_PAIR(player_def->color_pair) | A_BOLD);
                mvaddch(screen_y, screen_x, player_def->character);
                attroff(COLOR_PAIR(player_def->color_pair) | A_BOLD);
            }
        }
    }

    // Display info
    attron(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK));
    mvprintw(0, 0, "Position: (%.1f, %.1f) | Grid: (%d, %d) | Entities: %d | q to quit",
             player->pos.x, player->pos.y,
             static_cast<int>(std::round(player->pos.x)), static_cast<int>(std::round(player->pos.y)),
             entity_count);
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK));

    // Show entity info at player's position
    Entity* at_player = find_entity_at(player->pos.x, player->pos.y);
    if (at_player && at_player != player) {
        EntityDefinition* at_def = get_entity_definition(at_player->definition_id);
        if (at_def) {
            attron(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
            mvprintw(1, 0, "Here: %s (%c) - %s",
                     at_def->name.empty() ? "Unknown" : at_def->name.c_str(),
                     at_def->character,
                     at_def->passable ? "passable" : "impassable");
            attroff(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
        }
    }

    refresh();
}

// ============================================================================
// Sample World Creation
// ============================================================================

void create_sample_world() {
    // Create some scenery (trees)
    create_entity(-5.0, 3.0, 'T', COLOR_PAIR_GREEN_BLACK, EntityType::SCENERY, true, "Pine Tree");
    create_entity(-3.0, 4.0, 'T', COLOR_PAIR_GREEN_BLACK, EntityType::SCENERY, true, "Oak Tree");
    create_entity(4.0, -2.0, 'T', COLOR_PAIR_GREEN_BLACK, EntityType::SCENERY, true, "Maple Tree");

    // Create some walls (impassable)
    for (int x = -2; x <= 2; x++) {
        create_entity(static_cast<double>(x), 5.0, '#', COLOR_PAIR_WHITE_BLACK, EntityType::SCENERY, false, "Stone Wall");
    }
    for (int y = -3; y <= 3; y++) {
        create_entity(7.0, static_cast<double>(y), '#', COLOR_PAIR_WHITE_BLACK, EntityType::SCENERY, false, "Stone Wall");
    }

    // Create some items (passable)
    create_entity(-4.0, -1.0, '$', COLOR_PAIR_YELLOW_BLACK, EntityType::ITEM, true, "Gold Coin");
    create_entity(2.0, 2.0, '!', COLOR_PAIR_MAGENTA_BLACK, EntityType::ITEM, true, "Health Potion");
    create_entity(-1.0, -3.0, '/', COLOR_PAIR_CYAN_BLACK, EntityType::ITEM, true, "Sword");

    // Create some mobs (impassable)
    create_entity(-6.0, -4.0, 'g', COLOR_PAIR_GREEN_BLACK, EntityType::MOB, false, "Goblin");
    create_entity(5.0, 4.0, 'o', COLOR_PAIR_RED_BLACK, EntityType::MOB, false, "Orc");

    // Create environmental hazards (passable but dangerous-looking)
    create_entity(0.0, -5.0, '^', COLOR_PAIR_WHITE_RED, EntityType::ENVIRONMENTAL, true, "Fire");
    create_entity(3.0, -4.0, '~', COLOR_PAIR_GREEN_BLACK, EntityType::ENVIRONMENTAL, true, "Acid Pool");

    // Create a pet (passable)
    create_entity(1.0, 1.0, 'd', COLOR_PAIR_YELLOW_BLACK, EntityType::PET, true, "Dog");
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
    Entity* player = create_entity(0.0, 0.0, '@', COLOR_PAIR_WHITE_BLACK, EntityType::PLAYER, false, "Player");

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
    for (int i = 0; i < MAX_ENTITY_DEFINITIONS; i++) {
        if (entity_definitions[i].active) {
            destroy_entity_definition(&entity_definitions[i]);
        }
    }

    endwin();

    return 0;
}
