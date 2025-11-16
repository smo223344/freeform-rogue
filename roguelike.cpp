#include <ncurses.h>
#include <locale.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include "entity.h"
#include "mob.h"
#include "combat.h"

// Global MOB storage
std::vector<MOB*> mobs;

// Message log
constexpr int MAX_LOG_MESSAGES = 100;
std::vector<std::string> message_log;

void add_message(const std::string& message) {
    message_log.push_back(message);
    // Keep only the last MAX_LOG_MESSAGES messages
    if (message_log.size() > MAX_LOG_MESSAGES) {
        message_log.erase(message_log.begin());
    }
}

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

void draw_status_panel(Entity* player) {
    if (!player || !player->mob) return;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Status panel on the right side
    const int panel_width = 25;
    const int panel_x = max_x - panel_width;

    // Draw border
    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK) | A_BOLD);
    for (int y = 0; y < max_y; y++) {
        mvaddch(y, panel_x - 1, '|');
    }
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK) | A_BOLD);

    // Player name/title
    attron(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK) | A_BOLD);
    mvprintw(1, panel_x + 1, "=== PLAYER ===");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK) | A_BOLD);

    // HP display with bar
    int hp_percent = (player->mob->hp * 100) / player->mob->max_hp;
    int bar_width = 15;
    int filled = (hp_percent * bar_width) / 100;

    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(3, panel_x + 1, "Health:");
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Color HP bar based on percentage
    int hp_color = COLOR_PAIR_GREEN_BLACK;
    if (hp_percent < 30) {
        hp_color = COLOR_PAIR_RED_BLACK;
    } else if (hp_percent < 60) {
        hp_color = COLOR_PAIR_YELLOW_BLACK;
    }

    attron(COLOR_PAIR(hp_color) | A_BOLD);
    mvprintw(4, panel_x + 1, "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            addch('=');
        } else {
            addch(' ');
        }
    }
    addch(']');
    attroff(COLOR_PAIR(hp_color) | A_BOLD);

    // HP numbers
    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(5, panel_x + 1, "%d / %d", player->mob->hp, player->mob->max_hp);
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Action Points
    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(7, panel_x + 1, "Action Points:");
    mvprintw(8, panel_x + 1, "%.0f", player->mob->action_points);
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Position
    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(10, panel_x + 1, "Position:");
    mvprintw(11, panel_x + 1, "X: %.1f", player->pos.x);
    mvprintw(12, panel_x + 1, "Y: %.1f", player->pos.y);
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Faction
    const char* faction_name = "Unknown";
    if (player->mob->faction == FACTION_PLAYER) {
        faction_name = "Player";
    } else if (player->mob->faction == FACTION_NEUTRAL) {
        faction_name = "Neutral";
    } else if (player->mob->faction == FACTION_HOSTILE) {
        faction_name = "Hostile";
    }

    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(14, panel_x + 1, "Faction:");
    mvprintw(15, panel_x + 1, "%s", faction_name);
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Controls at bottom of panel
    attron(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));
    mvprintw(max_y - 6, panel_x + 1, "--- CONTROLS ---");
    attroff(COLOR_PAIR(COLOR_PAIR_YELLOW_BLACK));

    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    mvprintw(max_y - 4, panel_x + 1, "WASD/Arrows: Move");
    mvprintw(max_y - 3, panel_x + 1, "Bump: Attack");
    mvprintw(max_y - 2, panel_x + 1, "Q: Quit");
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
}

void draw_map(Entity* player) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    clear();

    // Status panel dimensions
    const int panel_width = 25;
    const int panel_x = max_x - panel_width;
    const int map_max_x = panel_x - 1;  // Don't draw into the panel area

    // Draw coordinate axes (optional, for visual reference)
    int center_x = max_x / 2;
    int center_y = max_y / 2;

    attron(COLOR_PAIR(COLOR_PAIR_BLUE_BLACK) | A_DIM);
    // Draw horizontal axis (but not in the panel area)
    for (int x = 0; x < map_max_x; x++) {
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
    for (auto& e : entities) {
        if (!e.active || &e == player) continue;

        EntityDefinition* def = get_entity_definition(e.definition_id);
        if (!def) continue;  // Skip if definition missing

        int screen_x, screen_y;
        world_to_screen(e.pos.x, e.pos.y, &screen_x, &screen_y);

        // Don't draw in the status panel area
        if (screen_x >= 0 && screen_x < map_max_x && screen_y >= 0 && screen_y < max_y) {
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

            // Don't draw in the status panel area
            if (screen_x >= 0 && screen_x < map_max_x && screen_y >= 0 && screen_y < max_y) {
                attron(COLOR_PAIR(player_def->color_pair) | A_BOLD);
                mvaddch(screen_y, screen_x, player_def->character);
                attroff(COLOR_PAIR(player_def->color_pair) | A_BOLD);
            }
        }
    }

    // Display simple title bar
    attron(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK) | A_BOLD);
    mvprintw(0, 1, "Roguelike Dungeon");
    attroff(COLOR_PAIR(COLOR_PAIR_CYAN_BLACK) | A_BOLD);

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

    // Display message log at the bottom of the screen (but not in panel area)
    const int log_lines = 5;  // Show last 5 messages
    int log_start_y = max_y - log_lines;
    int msg_index = 0;

    // Calculate which messages to show (last log_lines messages)
    int start_msg = static_cast<int>(message_log.size()) - log_lines;
    if (start_msg < 0) start_msg = 0;

    attron(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));
    for (size_t i = start_msg; i < message_log.size(); i++) {
        // Clear the line first to avoid leftover text
        move(log_start_y + msg_index, 0);
        clrtoeol();
        mvprintw(log_start_y + msg_index, 0, "%s", message_log[i].c_str());
        msg_index++;
    }
    attroff(COLOR_PAIR(COLOR_PAIR_WHITE_BLACK));

    // Draw status panel on the right side
    draw_status_panel(player);

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

    // Create some mobs (impassable) with AI
    Entity* goblin = create_entity(-6.0, -4.0, 'g', COLOR_PAIR_GREEN_BLACK, EntityType::MOB, false, "Goblin");
    if (goblin) {
        goblin->mob = new Enemy(goblin, 50, 80.0);  // 50 HP, 80 AP per turn
        mobs.push_back(goblin->mob);
    }

    Entity* orc = create_entity(5.0, 4.0, 'o', COLOR_PAIR_RED_BLACK, EntityType::MOB, false, "Orc");
    if (orc) {
        orc->mob = new Enemy(orc, 80, 70.0);  // 80 HP, 70 AP per turn (slower but tougher)
        mobs.push_back(orc->mob);
    }

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
    // Set locale for Unicode/wide character support
    setlocale(LC_ALL, "");

    // Seed random number generator for combat
    std::srand(static_cast<unsigned>(std::time(nullptr)));

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
    if (player) {
        player->mob = new Player(player);
        mobs.push_back(player->mob);
    }

    // Create sample world
    create_sample_world();

    // Welcome message
    add_message("Welcome to the dungeon!");
    add_message("Use WASD or arrow keys to move. Attack enemies by bumping into them.");

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

        // Process movement/attack if player tried to move
        if (new_x != player->pos.x || new_y != player->pos.y) {
            if (player->mob) {
                // Check if there's an entity at the target position
                Entity* target_entity = find_entity_at(new_x, new_y);
                bool took_action = false;

                // If target has a MOB and it's hostile to the player, attack it!
                if (target_entity && target_entity->mob && target_entity != player &&
                    player->mob->is_hostile_to(target_entity->mob)) {
                    if (player->mob->can_act(MOB::ATTACK_COST)) {
                        // Get target name for logging
                        EntityDefinition* target_def = get_entity_definition(target_entity->definition_id);
                        std::string target_name = (target_def && !target_def->name.empty()) ?
                                                   target_def->name : "Enemy";

                        CombatResult result = Combat::resolve_attack(player->mob, target_entity->mob);
                        player->mob->spend_ap(MOB::ATTACK_COST);
                        took_action = true;

                        // Log combat results
                        if (!result.hit) {
                            add_message("You miss the " + target_name + "!");
                        } else if (result.critical) {
                            add_message("Critical hit! You deal " + std::to_string(result.damage_dealt) +
                                       " damage to the " + target_name + "!");
                        } else {
                            add_message("You hit the " + target_name + " for " +
                                       std::to_string(result.damage_dealt) + " damage.");
                        }

                        // If we killed the target, destroy its entity and log it
                        if (result.target_killed && target_entity->mob) {
                            add_message("The " + target_name + " is defeated!");
                            destroy_entity(target_entity);
                        }
                    }
                }
                // Otherwise, try to move
                else if (player->mob->can_act(MOB::MOVE_COST)) {
                    if (is_position_passable(new_x, new_y, player)) {
                        player->pos.x = new_x;
                        player->pos.y = new_y;
                        player->mob->spend_ap(MOB::MOVE_COST);
                        took_action = true;
                    }
                }

                // If player took an action, process enemy turns
                if (took_action) {
                    for (auto* mob : mobs) {
                        if (mob != player->mob && mob->is_alive()) {
                            mob->take_turn();
                        }
                    }
                }
            }
        }

        // Give player action points each frame
        if (player->mob) {
            player->mob->accumulate_ap();
        }

        draw_map(player);
    }

    // Clean up
    // Delete all MOBs
    for (auto* mob : mobs) {
        delete mob;
    }
    mobs.clear();

    // Clean up entities
    for (auto& e : entities) {
        if (e.active) {
            destroy_entity(&e);
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
