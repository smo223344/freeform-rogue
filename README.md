# NCurses Roguelike

A simple ncurses-based roguelike game with floating-point position tracking.

## Features

- Floating-point position system (positions stored as doubles)
- Grid-based rendering (draws characters at nearest grid location)
- Centered coordinate system (0,0 is at the center of the screen)
- Arrow key movement (moves by 1.0 unit per keypress)
- Visual coordinate axes for reference

## Building

```bash
make
```

## Running

```bash
./roguelike
# or
make run
```

## Controls

- **Arrow Keys**: Move the player character
  - Up: Move +1.0 in Y axis
  - Down: Move -1.0 in Y axis
  - Left: Move -1.0 in X axis
  - Right: Move +1.0 in X axis
- **q**: Quit the game

## Implementation Details

- Player position is stored as floating-point values (double precision)
- The screen center represents world coordinates (0.0, 0.0)
- Positions are rounded to nearest integer for grid-based rendering
- The Y axis is inverted (positive Y is up) for more intuitive movement
- The '@' character represents the player

## Requirements

- ncurses library
- C compiler (gcc)
- math library
