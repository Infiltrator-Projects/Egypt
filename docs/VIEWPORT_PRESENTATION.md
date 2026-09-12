# Egypt — Viewport Presentation Rules

Status: **Authoritative presentation rule captured from Pharaoh forensic comparison, 12 Sep 2026**

The logical city simulation uses a finite rectangular tile grid. That implementation detail must not be presented to the player as a giant isometric diamond, triangle, sawtooth boundary or exposed board edge.

## Normal city view

- The camera viewport must read as a continuous landscape.
- Raw logical-map edges are not normal scenery.
- The renderer may draw non-interactive presentation terrain beyond the authoritative simulation grid so the viewport remains visually filled.
- Presentation terrain outside the grid is not buildable and is not simulation state.
- If a scenario needs a visible world boundary later, it must be an authored geographic/gameplay boundary (desert, cliffs, water, inaccessible edge treatment, etc.), not the accidental polygon created by isometric projection.
- Camera constraints may also keep the playable grid appropriately framed, but must not reveal an implementation-coloured void.

## UI compositing

The world is rendered first. HUD, panels, minimap, inspector and construction controls are composited afterwards.

No terrain tile, building, cloud shadow, walker, cart or other world geometry may overwrite the HUD. The previous renderer drew the top HUD first and then drew terrain over it, producing the visible zig-zag/sawtooth edge seen in the 12 Sep 2026 trial screenshot. That draw order is explicitly forbidden.

## Reference observation

The Pharaoh reference screenshots show the logical grid only through placement/flat-mode information. Normal play does not expose a mathematical diamond-shaped map board. Terrain fills the usable viewport and the UI remains cleanly layered over the world.
