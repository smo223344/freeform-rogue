#include <ncurses.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
    double x;
    double y;
} Position;

// Convert world coordinates to screen coordinates
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

void draw_map(Position player_pos) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    clear();

    // Draw coordinate axes (optional, for visual reference)
    int center_x = max_x / 2;
    int center_y = max_y / 2;

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

    // Draw player
    int screen_x, screen_y;
    world_to_screen(player_pos.x, player_pos.y, &screen_x, &screen_y);

    if (screen_x >= 0 && screen_x < max_x && screen_y >= 0 && screen_y < max_y) {
        mvaddch(screen_y, screen_x, '@');  // Using @ for smiley for now
    }

    // Display position info
    mvprintw(0, 0, "Position: (%.1f, %.1f) | Grid: (%d, %d) | q to quit",
             player_pos.x, player_pos.y, (int)round(player_pos.x), (int)round(player_pos.y));

    refresh();
}

int main() {
    Position player_pos = {0.0, 0.0};

    // Initialize ncurses
    initscr();
    cbreak();              // Disable line buffering
    noecho();              // Don't echo keypresses
    keypad(stdscr, TRUE);  // Enable arrow keys
    curs_set(0);           // Hide cursor

    // Main game loop
    int running = 1;
    draw_map(player_pos);

    while (running) {
        int ch = getch();

        switch (ch) {
            case KEY_UP:
                player_pos.y += 1.0;
                break;
            case KEY_DOWN:
                player_pos.y -= 1.0;
                break;
            case KEY_LEFT:
                player_pos.x -= 1.0;
                break;
            case KEY_RIGHT:
                player_pos.x += 1.0;
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
        }

        draw_map(player_pos);
    }

    // Clean up
    endwin();

    return 0;
}
