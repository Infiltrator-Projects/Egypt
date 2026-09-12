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
- asset-driven graphical presentation and UI composited into the framebuffer;
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

## TD-008 — The game is not a window-system game

**Date:** 2026-09-12

**Status:** DECIDED / ENFORCED

Egypt's simulation, renderer, framebuffer, camera, UI and input model are platform-neutral game code. They must not include or expose X11, Wayland, Win32, Cocoa or any other operating-system/window-system API types.

Platform code exists only at the outer host boundary. The host adapter is responsible for obtaining a display surface, translating native keyboard/mouse events into Egypt's own `Key` and `MouseButton` values, presenting Egypt's framebuffer, and passing resize/quit requests across the boundary.

The current Linux development adapter happens to be X11, but X11 is not part of the engine architecture. It lives only in `src/platform/x11_backend.cpp` and can be replaced by another host adapter without changing the game, simulation or renderer.

Build rules enforce this boundary:

- `EGYPT_HOST_BACKEND=x11` builds the current Linux desktop host adapter;
- `EGYPT_HOST_BACKEND=none` configures and builds the platform-neutral core with no X11 requirement at all;
- `egypt-game-core-compile` includes the complete `Game`/renderer/UI headers without linking or including X11;
- CI has a dedicated no-window-system job so platform APIs leaking back into game code break the build immediately.

A future Linux fullscreen/direct-display adapter may use DRM/KMS and input devices directly; Windows can use its own native adapter. Those choices must remain implementation details outside the game core.

## TD-009 — Everything visible is graphics in the game framebuffer

**Date:** 2026-09-12

**Status:** DECIDED / AUTHORITATIVE

Egypt is a pixel-graphics game. Everything the player sees must ultimately be graphical pixels composed by the game into its own framebuffer. The presentation model is deliberately closer to classic Amiga-style graphics programming than to a text-oriented desktop application or DOS-style control interface.

The game composes a complete frame in memory. Terrain, buildings, citizens, effects, HUD panels, buttons, icons, numbers, labels, dates and all other visible elements are drawn or blitted into that frame. The platform/host layer's job is only to present the completed pixel buffer and provide input/events. It must not be responsible for drawing player-facing UI or text.

Normal rendering should use a front/back-buffer style model: while one completed image is being presented, the game prepares the next complete image in another buffer or equivalent presentation surface, then presents/swaps the completed frame. The exact host implementation may differ by platform, but that must not alter the game's all-pixel presentation model.

### Text is graphics

There is no separate visual "text mode" in Egypt. Player-visible text is graphical artwork.

- A letter, digit or punctuation mark is a glyph image/sprite/bitmap asset.
- Drawing `JAN 3500 BC`, population totals or a menu label means compositing graphical glyphs into the framebuffer.
- The operating system's text widgets, terminal rendering, desktop fonts or native controls are not part of the game's visual presentation.
- Typography may originate from a designed typeface during asset production, but the runtime result used by the game is graphical glyph artwork/atlas data rendered as pixels.
- Text must visually belong to the artwork around it rather than look like console, terminal, system-widget or enlarged 5×7 bitmap output.

### UI is artwork, not programmer primitives

Production player-facing UI must be asset-driven artwork rather than a collection of primitive rectangles, lines and triangles pretending to be finished graphics.

Examples include:

- HUD bars and frames as authored graphical assets;
- speed controls as graphical button/medallion sprites with state variants;
- icons as authored graphical sprites;
- date plaques, menu plaques, inspector frames and decorative borders as graphical assets;
- graphical glyph atlases for labels and changing numerical values;
- state changes implemented by selecting/compositing the appropriate graphical asset, not by falling back to native widgets or text-mode styling.

Framebuffer primitives remain useful internally for rasterisation, masks, selection outlines, diagnostics, placement previews and temporary development scaffolding. They are not the target visual language for finished player-facing HUD artwork.

### Visual prohibition

The normal game must not drift toward a CLI, terminal, DOS utility, native desktop form or debug-tool appearance. In particular, large scaled 5×7 bitmap lettering, generic outlined rectangular buttons, system-looking controls and procedurally assembled pseudo-ornament are not acceptable substitutes for final graphical assets.

When the Pharaoh/reference material clearly shows an element as illustrated UI artwork, Egypt should treat that evidence as authoritative for the presentation approach: build an original graphical asset serving the same visual/function role rather than approximating it with text and geometric primitives.
