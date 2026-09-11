# Egypt — Technical Decisions

Status: **Living implementation record**

This file records implementation choices separately from the game-design specification.

## TD-001 — No third-party game engine

**Date:** 2026-09-11

**Status:** DECIDED

**Decision:**

Egypt will not use Godot, Unity, Unreal, or another third-party game engine as its foundation.

The project is to be built as our own engine and game codebase from the ground up. Third-party infrastructure may only be used where it does not impose ownership claims, royalties, control, forced branding, or game-engine dependence.

For the bootstrap, the implementation target is:

- C++20
- our own application/game loop
- our own software framebuffer renderer
- our own menu/UI hit-testing and state handling
- our own world/simulation state
- native Linux platform/window integration first
- Windows platform integration later behind the same platform abstraction

Operating-system APIs, the compiler, standard C/C++ libraries and low-level graphics/window interfaces are infrastructure rather than game engines.

## TD-002 — First milestone is visual

**Status:** DECIDED

Before production chains, economy, walkers or Nile simulation, the project must launch into a recognisable start screen and be able to enter a city screen.

The first executable milestone is intentionally tiny:

1. launch Egypt;
2. show an original Egyptian-themed main menu;
3. click **New Game**;
4. enter a primitive Nile/desert city screen;
5. return to the menu or quit cleanly.

The game can do almost nothing beyond that at first. The purpose is to establish our executable, renderer, input path and screen-state architecture before adding simulation complexity.

## TD-003 — Previous Godot bootstrap rejected

**Status:** REJECTED / SUPERSEDED

A brief Godot bootstrap was committed while exploring the fastest route to a start screen. That direction was rejected before becoming part of the architecture.

No future Egypt implementation should depend on Godot files, GDScript, Godot scenes, Godot runtime behaviour, or Godot packaging.
