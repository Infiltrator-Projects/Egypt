# Egypt — Bootstrap

## First executable milestone

The first goal is deliberately small and visual:

> **Launch Egypt, reach a recognisable start screen, press New Game, and enter a primitive city screen even though the simulation does almost nothing yet.**

## Implementation

Egypt now begins as a native C++20 application rather than using a third-party game engine.

The first shell contains:

- our own application/game state loop;
- our own CPU framebuffer;
- our own rectangle, line and triangle drawing;
- our own 5×7 bitmap font renderer;
- our own menu button hit-testing and screen state;
- a native Linux X11 presentation/input layer;
- an Egyptian-themed start screen drawn entirely from our code;
- New Game → primitive Nile/floodplain/desert city screen;
- disabled Continue button until save support exists;
- Settings placeholder;
- Quit and Escape handling;
- a basic HUD with population, treasury, year and flood forecast placeholders.

No Godot project, GDScript, Godot scene or Godot runtime is part of the active implementation.

## Build

On a Linux development system with a C++20 compiler and X11 development headers available:

```text
make
./build/egypt
```

The build does not package a third-party game engine or runtime with Egypt. The current Linux executable uses the display facilities already installed on the host system.

## Next smallest milestone

Do not add the economy yet. Make the city screen feel like a builder first:

1. camera pan and zoom;
2. explicit world/map data separate from rendering;
3. road placement tool;
4. place/remove road cells;
5. inspect a cell;
6. then add housing.

After that, build the first complete economic proof:

**clay pit → clay transport → potter → pottery → market/storage → house**.
