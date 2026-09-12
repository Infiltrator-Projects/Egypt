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
- minimap supports click repositioning; drag navigation remains a refinement;
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

Clicking a house now opens a substantially richer diagnostic panel than the original bootstrap inspector.

Current information includes:

- housing type/level;
- occupants / capacity / spare capacity;
- employed/free residents;
- food stock;
- pottery stock;
- water service;
- road access;
- desirability;
- plain-language next-evolution blocker.

Later systems should add taxation, health, fire, collapse, crime and further household goods only when those simulations genuinely exist rather than showing fake placeholder status.

Current evolution diagnostics include road, food, water, pottery and desirability/service blockers.

## Roads

Roads are simulation state, not static paint.

Current bootstrap progression:

**New Track → Worn Road → Established Road**

Actual immigrants, workers and logistics agents add traffic as they traverse road tiles. Traffic state is persistent simulation data and now changes the rendered appearance of the route. Future systems may use traffic for travel speed, congestion or maintenance.

## Food and storage

Current chain:

**farm/hunting lodge → physical food cart → granary → physical food cart → market → physical food cart → house**

Food no longer transfers instantaneously between these stages. Dispatch removes stock from the source, creates a `GoodsAgent`, routes it over the actual road network, and deposits the load only when the agent arrives. Food in transit remains part of the city's total-food accounting and its movement contributes road traffic.

Granaries, markets, farms, hunting lodges and houses all hold real local food stock. A broken road now physically breaks the delivery chain rather than leaving an invisible city-wide supply connection.

The next refinement is employment: farms, granaries and markets should ultimately need real residents/workers rather than operating solely because they have road access.

## Employment and hunting

Current first visible employment slice:

**HOME → COMMUTING TO LODGE → HUNTER ROLE → HUNTING TRIP → RETURNING WITH MEAT → LODGE/STORAGE → HOME/REPEAT**

Workers belong to actual households and remain part of resident population. Road travel contributes traffic.

Future work should extend the same home/job relationship to farms, clay pits, potters, storage, markets and other workplaces instead of using a global magical labour pool.

## Clay and pottery — restored foundational milestone

The original first end-to-end industrial proof was always:

**clay pit → physical clay movement → potter → pottery → market/storage → physical household delivery → housing improvement**

The project had drifted away from this while food/hunting were being developed. The forensic pass restored it as an active simulation slice.

Current implementation includes a generic `GoodsAgent` used by food, clay and pottery logistics:

- clay pits produce finite clay stock;
- clay is removed from the source only when a physical goods agent is dispatched;
- the goods agent travels the actual road network to a potter;
- potters consume clay and create finite pottery stock;
- pottery physically travels from potter to market;
- pottery physically travels from market to a household;
- household pottery stock is consumed over time;
- pottery gates the first goods-dependent housing level;
- all logistics carts contribute road traffic while travelling.

The next refinement is to tie clay/pottery production to real commuting workers rather than production occurring merely because the building has road access.

## Desirability

A first bootstrap desirability score exists so housing diagnostics can distinguish pleasant and industrial neighbourhoods.

Current influences are intentionally simple/original and are not a copy of Pharaoh values. Water/reeds, wells and markets can help; clay pits, potters, hunting lodges and granaries can reduce nearby desirability. This is a foundation for later overlays and housing requirements, not final balancing.

## Calendar and simulation speed

Simulation time has an authoritative month/year derived from world ticks, beginning at 3500 BC for the bootstrap scenario. It is simulation state rather than decorative UI text.

The live HUD now exposes the date plus pause, x1, x2 and x4 controls. Space also toggles pause. The exact historically correct campaign calendar and scenario dates remain a later content/balance decision.

## Atmosphere

Current presentation now includes:

- moving cloud-shadow overlays crossing terrain/buildings;
- animated Nile surface detail;
- the existing visible immigrants, workers and carts.

Still required:

- actual wildlife agents rather than only the hunter abstraction;
- birds/ibis and other ambient creatures;
- richer vegetation and water animation;
- more natural cloud shapes/lighting after the renderer/art pipeline improves.

Ambient effects should remain lightweight and should not become authoritative gameplay state unless explicitly promoted into simulation later.

## Flat/diagnostic view and minimap

The Pharaoh reference showed that the flat footprint-style view is genuinely useful, not merely debug junk. Egypt now preserves this concept as a toggleable diagnostic mode (`F`).

The lower-left minimap now represents the whole simulation map and an approximate current camera viewport. Clicking the minimap recentres the main camera. Drag navigation and better viewport geometry remain refinements.

## Current implementation checkpoint

Implemented and tested in the headless world model:

- map terrain and Pharaoh-style isometric handedness;
- physical road topology;
- physical farm/hunting → granary → market → house food logistics;
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

Implemented in presentation:

- 2.5D isometric projection/picking;
- correct map handedness;
- edge-scroll, drag-pan, wheel zoom, keyboard pan and recenter;
- display settings bootstrap;
- procedural housing/well/hunting/granary/market/farm/industry assets;
- visible immigrants, hunters and food/clay/pottery carts;
- road appearance changes from measured traffic;
- expanded residence/road/industry inspector;
- month/year HUD plus pause/x1/x2/x4 controls;
- whole-map minimap with click navigation and approximate viewport;
- moving cloud shadows and animated water detail;
- flat/diagnostic footprint mode.

## High-priority presentation gaps

1. Replace procedural programmer art with production-quality original assets and animation.
2. Improve the house panel from a large diagnostic overlay into a polished game window with clearer iconography/grouping.
3. Improve minimap viewport accuracy and add click-drag navigation.
4. Add real birds/ibis, wildlife and richer ambient animation.
5. Improve cloud-shadow shapes and lighting once the graphics pipeline is less primitive.
6. Improve carts/people from coloured primitives into readable animated sprites/assets.

## High-priority simulation gaps

1. Tie farms, clay pits, potters, markets, granaries and future industry to actual labour/commuting.
2. Make 2x2 housing merge a genuine multi-tile residence identity rather than only a visual merge.
3. Replace abstract hunting targets with real wildlife agents.
4. Add causal supply tracing to the inspector: house ← market ← storage ← producer.
5. Add reeds → papyrus as the second manufacturing chain.
6. Begin Nile inundation/agricultural-cycle simulation after the basic logistics slice is physically coherent.
7. Add storage/distribution policies so the player can intentionally route goods rather than relying only on automatic nearest viable chains.

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
