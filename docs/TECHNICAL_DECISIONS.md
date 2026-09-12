# Egypt — Technical Decisions

Status: **Living implementation record**

This file records implementation choices separately from the game-design specification.

## TD-001 — No third-party game engine

**Date:** 2026-09-11

**Status:** DECIDED

Egypt will not use Godot, Unity, Unreal, or another third-party game engine as its foundation.

The project is to be built as our own engine and game codebase from the ground up.

Initial implementation direction:

- C++20;
- our own application/game loop;
- our own framebuffer renderer;
- our own drawing primitives and UI;
- our own input/state handling;
- our own world and simulation model;
- native desktop platform integration;
- Linux first, Windows afterwards behind the same platform boundary.

## TD-002 — No third-party release attribution/licence payload

**Status:** DECIDED

Egypt releases will not bundle a third-party game engine, third-party runtime package, third-party assets, or a `THIRD_PARTY_LICENSES.txt` attribution payload.

The game may call facilities already supplied by the target operating system, just as ordinary native applications do, but third-party components are not to be copied into or redistributed as part of an Egypt release unless this decision is deliberately revisited.

The Egypt repository itself will not be given an open-source licence unless Shannon explicitly chooses one later. In the absence of a repository licence grant, the project's source remains copyright-reserved by default.

## TD-003 — First milestone is visual

**Status:** DECIDED

Before production chains, economy, walkers or Nile simulation, the project must launch into a recognisable start screen and be able to enter a city screen.

The first executable milestone is intentionally tiny:

1. launch Egypt;
2. show an original Egyptian-themed main menu;
3. click **New Game**;
4. enter a primitive Nile/desert city screen;
5. return to the menu or quit cleanly.

The game can do almost nothing beyond that at first. The purpose is to establish our executable, renderer, input path and screen-state architecture before adding simulation complexity.

## TD-004 — Previous Godot bootstrap rejected

**Status:** REJECTED / SUPERSEDED

A brief Godot bootstrap was committed while exploring the fastest route to a start screen. That direction was rejected before becoming part of the architecture.

No future Egypt implementation should depend on Godot files, GDScript, Godot scenes, Godot runtime behaviour, or Godot packaging.

## TD-005 — Isometric 2.5D presentation

**Date:** 2026-09-12

**Status:** DECIDED

The city view will use an isometric 2.5D presentation.

The authoritative simulation remains a normal logical 2D world/grid. Rendering projects those coordinates into an angled isometric view. Terrain is represented as isometric tiles; structures, vegetation, citizens, carts, boats and effects may extend vertically above their ground tile and are depth-ordered in presentation space.

This deliberately preserves the readability and city-planning feel of classic isometric city builders without requiring a full free-rotation 3D engine.

Initial rendering rules:

- square logical map coordinates are authoritative;
- isometric projection is a presentation transform only;
- map-to-screen and screen-to-map transforms must both exist;
- camera pan and zoom are required;
- terrain, decals, structures, agents and overlays are separate conceptual layers;
- rendering order must account for tile depth and vertical height;
- arbitrary 3D camera rotation is not part of the initial design;
- future elevation is allowed without replacing the world model.

## TD-006 — Reuse Infiltratr Common before inventing local infrastructure

**Date:** 2026-09-12

**Status:** DECIDED

Egypt must use the shared Infiltratr Common library where Common already provides a suitable implementation. Game-specific code remains in Egypt; generally reusable primitives belong in Common.

Egypt currently pins Common 1.16.0. The first simulation loop uses Common's exact fixed-step scheduler rather than inventing a private accumulator.

When outside projects are researched, their code is reference material unless Shannon deliberately approves importing a compatible dependency. We may study architecture, algorithms and design patterns, then implement the required behaviour in Egypt/Common. We do not silently add an engine, runtime or third-party asset package merely because it would be convenient.

## TD-007 — Pharaoh-style camera and display controls are core UX

**Date:** 2026-09-12

**Status:** DECIDED

The city view must be continuously navigable. Camera movement is part of the core interaction model, not a debug convenience.

Required baseline controls:

- edge scrolling with configurable speed;
- right- or middle-mouse drag panning;
- mouse-wheel zoom;
- keyboard pan using arrows and WASD;
- a recenter command;
- display settings with selectable window resolutions;
- camera input must remain responsive independently of the simulation tick rate.

The map should visually read as continuous terrain rather than exposed isometric graph paper. Logical tiles remain authoritative, but normal play should not show strong tile borders except for selection, placement previews or overlays.
