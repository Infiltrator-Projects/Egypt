# Egypt — External Code Research for Fast Development

Status: **Reference-only research**

Purpose: use existing public projects to identify proven architecture and implementation patterns that can accelerate Egypt without quietly changing the project's engine/licensing decisions.

No external project code listed here has been copied into Egypt by this research pass. The useful ideas are being reimplemented in Egypt or Infiltratr Common.

## ProcIsoCity

Repository: `masterblaster1999/ProcIsoCity`

Useful observations:

- C++20 isometric city-builder starter architecture;
- deterministic procedural terrain;
- explicit world/simulation separation;
- map-to-pixel isometric transforms;
- small initial economy/population simulation;
- versioned save design;
- later roadmap uses cached terrain layers, chunking and multiple render layers.

Why not import it: the interactive application depends on raylib and the project is MIT-licensed. Egypt does not need raylib or a second engine/runtime layer. We can reproduce the small useful architectural ideas directly.

## Cytopia

Repository: `CytopiaTeam/Cytopia`

Useful observations:

- custom C++ isometric renderer;
- camera pan and zoom;
- conversion between isometric/world and screen coordinates;
- multiple tile height levels;
- screen-space tile picking;
- terrain and building rendering are treated as distinct concerns.

Why not import it: Cytopia is built around SDL2 and its own project architecture/assets. Egypt is retaining its own native renderer.

## MicropolisCore

Repository: `SimHacker/MicropolisCore`

Useful observations:

- city simulation core separated from graphical front ends;
- engine can run headless;
- simulation subsystems are split by responsibility rather than being fused into rendering;
- renderer/UI acts as a consumer of simulation state.

Why not import it: Micropolis derives from SimCity Classic and is GPL-licensed with additional project/name history. Egypt is an original game, not a Micropolis derivative. Its simulation separation is a useful architectural proof, not a code source for Egypt.

## IsoCity

Repository: `amilich/isometric-city`

Useful observations:

- pure Canvas isometric renderer rather than a conventional game engine;
- layer/depth sorting is treated explicitly;
- roads, buildings, pedestrians and vehicles all sit on the same logical grid;
- state management and simulation are separated from drawing.

Why not import it: it is TypeScript/Next.js and MIT-licensed; the implementation language and runtime are not Egypt's architecture.

## Isometric-Tilemaps

Repository: `Maximetinu/Isometric-Tilemaps`

Useful observation: the project demonstrates the practical value of treating vertical height as a separate sorting dimension in an isometric world. It is Unlicense, but the implementation is Unity-specific and therefore not directly useful to Egypt.

## Patterns adopted for Egypt

The research supports the following immediate implementation plan:

1. Keep the authoritative world as an ordinary square logical grid.
2. Project grid coordinates into isometric screen coordinates only at the rendering boundary.
3. Implement the inverse screen-to-grid transform for mouse picking.
4. Render in deterministic depth order and leave an explicit elevation term in the projection.
5. Separate terrain, structures, moving agents and overlays into layers as the renderer grows.
6. Keep simulation independent of the renderer so it can later run headless/tests.
7. Start with deterministic procedural terrain so gameplay systems can be exercised immediately without waiting for a map editor.
8. Use Infiltratr Common for reusable timing/math/platform primitives before writing local equivalents.
9. Build the smallest complete city interaction loop first, then extend it into the clay-to-pottery vertical slice.

## Immediate speed-run target

The next executable milestone is no longer a static city placeholder. `New Game` should provide:

- an isometric Nile/desert/floodplain map;
- camera pan and zoom;
- screen-to-tile picking;
- inspect tool;
- road placement;
- house placement;
- clay-pit placement on clay terrain;
- potter placement;
- bulldozing;
- treasury costs;
- houses gaining population only when they have road access;
- a fixed-step simulation driven by Infiltratr Common.

That gives Egypt a real playable world shell on which the pottery logistics chain can be built next.
