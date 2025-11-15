# NCurses Roguelike

A simple ncurses-based roguelike game with floating-point position tracking and a comprehensive entity system.

## Features

- **Floating-point position system** - Positions stored as doubles for smooth movement
- **Grid-based rendering** - Draws characters at nearest grid location
- **Centered coordinate system** - (0,0) is at the center of the screen
- **Arrow key movement** - Moves by 1.0 unit per keypress
- **Visual coordinate axes** - For reference and navigation
- **Entity system** - Supports multiple entity types with different properties
- **Color support** - Entities rendered with customizable foreground/background colors
- **Collision detection** - Impassable entities block movement

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

- **Arrow Keys** or **WASD**: Move the player character
  - Up/W: Move +1.0 in Y axis
  - Down/S: Move -1.0 in Y axis
  - Left/A: Move -1.0 in X axis
  - Right/D: Move +1.0 in X axis
- **Q**: Quit the game

## Entity System

The game features a comprehensive entity system with the following types:

### Entity Types

- **ENTITY_SCENERY** - Static environmental objects (trees, decorations)
- **ENTITY_ITEM** - Collectible or interactive items (gold, potions, weapons)
- **ENTITY_MOB** - Enemy creatures (goblins, orcs)
- **ENTITY_PET** - Friendly companions (dogs, cats)
- **ENTITY_PROJECTILE** - Moving objects like arrows or spells
- **ENTITY_ENVIRONMENTAL** - Hazards like fire, acid, or water
- **ENTITY_PLAYER** - The player character

### Entity Properties

Each entity has:
- **Position** - Floating-point (x, y) coordinates
- **Character** - Single ASCII character for display
- **Color Pair** - Foreground and background color combination
- **Type** - One of the entity types above
- **Passable** - Whether other entities can move through this position
- **Name** - Optional descriptive name

### Sample Entities in the Demo

- **Trees (T)** - Green scenery, passable
- **Walls (#)** - White scenery, impassable (blocks movement)
- **Gold ($)** - Yellow items, passable
- **Potions (!)** - Magenta items, passable
- **Weapons (/)** - Cyan items, passable
- **Mobs (g, o)** - Green/red enemies, impassable
- **Fire (^)** - Environmental hazard with red background, passable
- **Acid (~)** - Green environmental hazard, passable
- **Pet (d)** - Yellow companion, passable
- **Player (@)** - White, bold, impassable

## Implementation Details

- Player position is stored as floating-point values (double precision)
- The screen center represents world coordinates (0.0, 0.0)
- Positions are rounded to nearest integer for grid-based rendering
- The Y axis is inverted (positive Y is up) for more intuitive movement
- Entity pool supports up to 1000 active entities
- Collision detection prevents movement into impassable entities
- Entities at the player's position are displayed in the UI

## Requirements

- ncurses library
- C compiler (gcc)
- math library
