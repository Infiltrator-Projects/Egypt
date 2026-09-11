# Egypt — Bootstrap

## First executable milestone

The first development goal is deliberately visual and small:

> **Launch Egypt and reach a recognisable game start screen, then press New Game and enter a city screen even if the simulation does almost nothing.**

This creates immediate visible progress and a stable shell into which the simulation can grow.

## Bootstrap technology

For this first executable slice:

- **Engine:** Godot 4.7.2 stable
- **Language:** GDScript
- **Renderer:** GL Compatibility during bootstrap
- **Logical viewport:** 1280 × 720, stretchable
- **Platforms:** desktop-first; Linux is the immediate development target, while Windows remains an intended export target

Godot/GDScript is the initial implementation stack because it provides fast visual iteration. The core simulation must remain separated from presentation so rendering/UI code never becomes the authoritative game state.

## Milestone 0.1 — Start screen

Implemented in the first code commit:

- project launches directly to an Egypt main menu;
- original placeholder Egyptian visual treatment drawn entirely in code;
- New Game opens a first city/map placeholder;
- Continue exists but is disabled until save support exists;
- Settings acknowledges the action but is intentionally not implemented yet;
- Quit exits the game;
- Escape returns from the city placeholder to the main menu;
- city placeholder exposes population, treasury, year and flood-forecast locations in the HUD;
- no external/copyrighted Pharaoh assets are required.

## Next smallest goal

Do not add the economy yet.

The next milestone should make the city screen interactive enough to feel like a builder:

1. camera pan/zoom;
2. visible build grid;
3. road tool;
4. place/remove road cells;
5. inspect a cell;
6. keep the map representation in simulation state rather than the renderer.

Once road placement feels good, housing and the clay → pottery chain can be layered onto it.
