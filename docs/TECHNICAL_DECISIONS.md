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
