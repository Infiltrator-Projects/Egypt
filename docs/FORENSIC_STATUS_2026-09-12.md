# Egypt — Forensic Reconstruction Status — 12 Sep 2026

This document consolidates the concrete design and implementation decisions recovered from the September 2026 Pharaoh reference walkthrough and the Egypt development discussion. It exists so that the project does not depend on chat history.

Where this file conflicts with older documents that still mark a decision as TBD, this file and `PHARAOH_FORENSIC_REFERENCE.md` describe the current direction unless a later explicit decision changes it.

## Locked project/technology direction

- Native C++20 game code.
- Custom engine/runtime rather than Godot or another third-party game engine.
- Infiltratr Common **1.16.0**, pinned to the reviewed release commit, is used for genuinely reusable infrastructure.
- Simulation remains authoritative and testable without rendering.
- Current Linux presentation backend is X11; renderer/platform backends may be replaced later without changing simulation rules.
- Main gameplay presentation is **2.5D isometric** over a logical 2D simulation grid.
- Pharaoh-style screen handedness is locked: logical +X projects down-left, logical +Y down-right.
- Camera angle is fixed/orthographic in spirit; pan and zoom are core controls. Arbitrary 3D rotation is not a requirement.

## Visual/readability target

The visual target is a modern original city-builder that preserves the useful readability of classic Pharaoh-style presentation without copying its assets/UI/content.

Required qualities recovered from the walkthrough:

- continuous-looking terrain rather than obvious diamond graph paper;
- roads that read as connected routes;
- buildings with readable height and footprint;
- small visible walkers, workers, immigrants, carts and wildlife;
- upgraded housing that visibly changes form;
- multi-tile residences/neighbourhoods that visually knit together;
- moving water detail, vegetation and ambient life;
- moving cloud shadows as a lightweight atmospheric layer;
- a useful flat/diagnostic footprint mode must remain available even when final art improves.

## Navigation and primary HUD

Established requirements:

- edge scrolling;
- right/middle mouse drag panning;
- mouse-wheel zoom;
- WASD/arrows panning;
- Home/recentre;
- configurable scroll speed;
- selectable display/window sizes;
- whole-map minimap in the lower-left;
- minimap shows the current camera viewport;
- minimap should eventually support click/drag repositioning;
- pause and multiple simulation-speed controls;
- month/year visible in the primary HUD;
- population and treasury visible in the primary HUD.

## Visible-causality rule

The governing rule is:

> Important state changes should have a visible cause and visible consequence in the city wherever practical.

Consequences of this rule already established:

- immigrants must enter the map and physically travel to housing;
- adding viable housing creates additional visible immigration traffic;
- population increments only when immigrants arrive;
- workers originate from real populated housing and commute to jobs;
- specialist workers visibly change role/appearance;
- hunters visibly leave the workplace, hunt, return with meat and contribute stock;
- goods should be physically located and physically moved;
- houses visibly improve when requirements are sustained;
- road use is recorded from actual traffic rather than a hidden arbitrary upgrade timer.

## Housing and immigration

Current bootstrap ladder:

1. **Hut** — capacity 8.
2. **Watered Home** — capacity 12.
3. **Established Residence** — capacity 16.
4. **Courtyard Home** — capacity 20; first goods-gated level requiring pottery.

The exact names/capacities are original bootstrap values and may be rebalanced.

Behaviour:

- road access alone never creates population;
- food eligibility creates housing demand;
- the road network must connect to a valid map edge/kingdom route;
- immigrants reserve target capacity while in transit;
- water from a nearby well allows early housing evolution;
- pottery is the first manufactured household good used for later evolution;
- upgraded housing creates extra capacity and therefore can trigger another immigration wave;
- sustained loss of requirements can cause regression;
- higher housing eventually depends on desirability and additional goods/services.

## House inspection

Clicking a house should eventually open a substantial diagnostic panel rather than a tiny debug readout.

The panel grows only as real systems exist and should expose:

- housing type/level;
- occupants / capacity / spare capacity;
- employed/free residents;
- food stock;
- pottery and future household goods;
- water service;
- road access;
- desirability;
- taxation when implemented;
- health/fire/collapse/crime only when those simulations genuinely exist;
- plain-language next-evolution blocker.

Current evolution diagnostics include road, food, water, pottery and desirability/service blockers.

## Roads

Roads are simulation state, not static paint.

Current bootstrap progression:

**New Track → Worn Road → Established Road**

Actual immigrants, workers and logistics agents add traffic as they traverse road tiles. Traffic state is persistent simulation data. Rendering should visibly reflect the three states; future systems may use traffic for travel speed, congestion or maintenance.

## Food and storage

Current chain:

**farm/hunting lodge → granary → market → house**

Current food movement between these stages is still partly logical/instant and is therefore an explicit unfinished item. The intended final rule is visible producer/storage/market transport with physical carts/workers.

Granaries and markets have real stock. Production without storage/distribution should eventually back up visibly instead of teleporting/discarding resources.

## Employment and hunting

Current first visible employment slice:

**HOME → COMMUTING TO LODGE → HUNTER ROLE → HUNTING TRIP → RETURNING WITH MEAT → LODGE/STORAGE → HOME/REPEAT**

Workers belong to actual households and remain part of resident population. Road travel contributes traffic.

Future work should extend the same home/job relationship to farms, clay pits, potters, storage, markets and other workplaces instead of using a global magical labour pool.

## Clay and pottery — restored foundational milestone

The original first end-to-end industrial proof was always:

**clay pit → physical clay movement → potter → pottery → market/storage → physical household delivery → housing improvement**

The project had drifted away from this while food/hunting were being developed. The forensic pass restores it as an active simulation slice.

Current implementation now includes a generic `GoodsAgent` for this chain:

- clay pits produce finite clay stock;
- clay is removed from the source only when a physical goods agent is dispatched;
- the goods agent travels the actual road network to a potter;
- potters consume clay and create finite pottery stock;
- pottery physically travels from potter to market;
- pottery physically travels from market to a household;
- household pottery stock is consumed over time;
- pottery gates the first goods-dependent housing level;
- goods carts contribute road traffic while travelling.

The next refinement is to tie clay/pottery production to real commuting workers rather than production occurring merely because the building has road access.

## Desirability

A first bootstrap desirability score now exists so housing diagnostics can begin distinguishing pleasant and industrial neighbourhoods.

Current influences are intentionally simple/original and are not a copy of Pharaoh values. Water/reeds, wells and markets can help; clay pits, potters, hunting lodges and granaries can reduce nearby desirability. This is a foundation for later overlays and housing requirements, not final balancing.

## Calendar

Simulation time now has an authoritative month/year derived from world ticks, beginning at 3500 BC for the bootstrap scenario. It is simulation state rather than decorative UI text.

The exact historically correct campaign calendar and scenario dates remain a later content/balance decision.

## Atmosphere

Recovered visual requirements:

- cloud shadows move across terrain/buildings;
- water surface has visible motion;
- birds/wildlife move independently of economic actors;
- vegetation contributes to a living landscape;
- ambient effects remain lightweight and are not confused with gameplay systems unless explicitly promoted into simulation later.

## Current implementation checkpoint

Implemented and tested in the headless world model:

- map terrain and Pharaoh-style isometric handedness;
- physical road topology;
- food production/storage/distribution bootstrap;
- visible immigrant simulation state;
- well service and housing evolution/capacity;
- labour tied to real residents for hunting lodges;
- visible hunter state machine;
- physical clay/pottery logistics agents;
- household pottery stock and pottery-gated housing evolution;
- persistent road traffic/evolution state;
- house evolution diagnostics;
- bootstrap desirability;
- simulation calendar;
- Common 1.16.0 fixed-step integration at application level;
- Linux CI building and running the world smoke test.

Implemented in presentation already:

- 2.5D isometric projection/picking;
- correct map handedness;
- edge-scroll, drag-pan, wheel zoom, keyboard pan and recenter;
- display settings bootstrap;
- procedural housing/well/hunting/granary/market/farm assets;
- visible immigrants and hunters;
- simple selected-tile inspector.

High-priority presentation gaps:

1. Draw road states differently from accumulated traffic.
2. Draw `GoodsAgent` carts/porters and carried clay/pottery.
3. Replace tiny house inspector with a real residence diagnostic window.
4. Show calendar in the HUD and add pause/multiple simulation speeds.
5. Add whole-map minimap with camera viewport and navigation.
6. Add cloud-shadow atmosphere and richer water/wildlife animation.
7. Add a flat/diagnostic footprint mode.
8. Replace procedural programmer art with production-quality original assets.

High-priority simulation gaps:

1. Convert food transfers to physical logistics agents.
2. Tie farms, clay pits, potters, markets, granaries and future industry to actual labour/commuting.
3. Make 2x2 housing merge a genuine multi-tile residence identity rather than only a visual merge.
4. Replace abstract hunting target with real wildlife agents.
5. Add pottery/storage causal tracing to the inspector.
6. Add reeds → papyrus as the second manufacturing chain.
7. Begin Nile inundation/agricultural-cycle simulation after the basic logistics slice is physically coherent.

## Guardrail

Before calling any new Egypt feature complete, check:

1. What caused this state change?
2. Is the cause visible or inspectable?
3. Is the resource/person physically located and moving where appropriate?
4. Can the player trace a broken chain backwards?
5. Does the simulation still work headlessly?
6. Are we using Common where the functionality is genuinely reusable?
7. Are the code, art, balance, names and content original?

If not, the feature is still scaffolding.
