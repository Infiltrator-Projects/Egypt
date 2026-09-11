# Egypt — Technical Decisions

Status: **Living implementation record**

This file records implementation choices separately from the game-design specification.

## TD-001 — Bootstrap engine and language

**Date:** 2026-09-11

**Status:** DECIDED for initial implementation

**Decision:**

- Godot 4.7.2 stable
- GDScript
- GL Compatibility renderer during early development
- desktop-first
- Linux as immediate development environment
- Windows kept as an intended export target

**Reason:** Reach a visible interactive game shell quickly without requiring an engine/toolchain project before the game itself exists.

**Architectural constraint:** Godot scene/UI nodes are presentation and interaction infrastructure. Authoritative simulation state must remain separable from rendering so core game logic can later be tested headlessly and scaled independently of graphics.

## TD-002 — First milestone is visual

**Status:** DECIDED

Before production chains, economy, walkers or Nile simulation, the project must launch into a recognisable start screen and be able to enter a city scene.

This is intentionally a psychological and technical milestone: the project should become a game-shaped executable immediately, then gain simulation depth incrementally.
