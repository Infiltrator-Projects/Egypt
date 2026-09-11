# Egypt — Design Decision Register

Status: **Authoritative project intent register**

This file exists to distinguish decisions that have actually been made from proposals, reference material and unanswered questions. It should be updated whenever a design choice becomes settled.

Terminology:

- **DECIDED** — part of the current project direction unless deliberately changed later.
- **DIRECTION** — desired direction, but details remain open.
- **PROPOSED** — worth exploring; not yet a requirement.
- **TBD** — deliberately undecided.
- **REJECTED** — something we specifically do not want to inherit or do by default.

---

## Project identity

### DECIDED — Public project repository

Repository: `Infiltrator-Projects/Egypt`

Working project name: **Egypt**.

The final commercial/game title is **TBD**.

### DECIDED — Inspiration without cloning

The primary reference is *Pharaoh* (1999) with *Cleopatra: Queen of the Nile*.

Egypt is **not** intended to be a remake, compatibility engine or clone. It must develop its own identity, simulation, code, art, UI, balance, campaign, maps and content.

Reference research is used to understand what worked and what should be redesigned.

---

## Core experience

### DECIDED — Watch the city work

The central design principle is:

> **If something important happens in the simulation, wherever practical the player should be able to see it happening in the city.**

Goods, services, workers, agriculture and construction should have visible consequences rather than existing only as counters.

### DECIDED — The simulation is authoritative

The renderer/UI presents the simulation. It must not be the source of truth for game state.

Simulation and presentation should be separated cleanly enough that game logic can be tested without rendering.

### DIRECTION — Large, busy cities

The game should eventually support substantially larger and more active settlements than were practical in 1999.

The exact population target, map dimensions and simulation granularity are **TBD**.

---

## Nile and environment

### DECIDED — The Nile is a game system

The Nile must not be decorative background.

Its water cycle should affect agriculture, floodplain fertility and city planning.

### DIRECTION — More visible inundation than Pharaoh

Desired behaviour includes visible water rise and retreat, changing floodplain state, fertility effects, irrigation and changing seasonal labour requirements.

Exact hydrology, calendar and historical abstraction remain **TBD**.

### DECIDED — Wildlife/ambient life belongs in the world

The map should contain ambient life appropriate to the setting, including crocodiles, hippopotamuses and birds where suitable.

### DECIDED — Ibis / "bin chickens" stay

This is a foundational project requirement.

---

## Economy and production

### DECIDED — Interconnected production chains

Industry should use comprehensible chains with shared inputs/by-products where useful.

Representative foundational chains include:

- reeds → papyrus;
- clay → pottery;
- clay + straw → bricks;
- grain → food + straw;
- barley → beer;
- flax → linen;
- stone → monumental/civic construction.

The exact complete resource list and recipes are **TBD**.

### DECIDED — Goods have physical location

Goods should normally exist somewhere in the city and need to be moved to where they are used.

Arbitrary resource teleportation between unrelated buildings is not the default model.

### DECIDED — Markets are part of logistics

Household distribution should involve real market inventory and delivery rather than a global citywide goods flag.

### DIRECTION — River freight matters

The Nile should eventually become part of the transport network, not only agricultural terrain.

Exact boat/freight implementation is **TBD**.

---

## Roads, walkers and services

### DECIDED — Visible walkers/agents remain important

People moving through the city are a core readability and atmosphere feature.

### REJECTED — Random roamer behaviour as the basis of service coverage

We do not want efficient cities to require artificial rectangular loops merely because service agents make arbitrary choices at intersections.

### DECIDED — Service agents should act purposefully

Agents should choose sensible targets according to their role while remaining constrained by road topology, travel time, congestion, capacity and access.

Examples:

- physicians prefer homes needing care;
- market distributors prefer undersupplied homes;
- fire services prioritise actual risk/incidents;
- buyers choose viable suppliers;
- delivery workers select sensible destinations.

### DIRECTION — Player routing controls

Service districts, gates, preferred freight routes and road restrictions are useful possibilities, but they should be tools rather than mandatory exploits.

Exact controls are **TBD**.

---

## Population and labour

### REJECTED — Pharaoh's recruiter fiction

A workplace should not become staffed because a special recruiter walker merely touched an arbitrary house while the actual labour effectively teleports from a global pool.

### DECIDED — Work must have a believable relationship to population

Employment should account for where labour lives and how it reaches jobs.

### DIRECTION — Commuting and labour accessibility

Distance, transport and local labour availability should matter. A remote quarry should therefore need an actual labour solution: nearby settlement, transport, work camp or equivalent.

Exact citizen granularity is **TBD**. Options include individuals, households, cohorts or a hybrid.

### DECIDED — Seasonal labour is important

Agriculture and major works should compete for labour according to season and demand.

This relationship is inspired by the original Work Camp concept but should be represented more clearly and believably.

---

## Housing and neighbourhoods

### DECIDED — Housing evolves rather than being directly upgraded by the player

The player creates conditions in which homes improve.

Housing should respond to access to goods, food, services, employment/prosperity and neighbourhood quality.

### DECIDED — Housing can decline

If important requirements disappear for long enough, neighbourhoods may deteriorate rather than remaining permanently upgraded.

### DIRECTION — Original housing ladder

Egypt will create its own progression and balancing rather than copying Pharaoh's exact sequence and thresholds.

Potential needs include water, food variety, pottery, beer, linen, religion, healthcare, education, entertainment, administration, luxury goods and desirability.

### DIRECTION — Social advancement should create trade-offs

A wealthy/educated elite can affect labour supply, taxation, administration and consumption rather than functioning as a pure numerical upgrade.

Details are **TBD**.

---

## Diagnostics and information

### DECIDED — Failure must be diagnosable

The player should be able to discover why a system is failing without trial-and-error guessing.

### DECIDED — Causal supply tracing

For a problem such as a house lacking pottery, the UI should be capable of exposing a chain similar to:

**House ← Market ← Storage ← Potter ← Clay Pit**

and showing where the bottleneck is.

### DIRECTION — Rich overlays

Useful overlays include goods, market coverage, water, health, fire, structural risk, employment/commute, desirability, religion, education, entertainment, tax, congestion, freight, fertility and irrigation.

Final UI design is **TBD**.

---

## Religion

### DECIDED — Religion is an important civic system

Religion should remain deeply connected to Egyptian identity and city life.

### REJECTED — Temple spam as the main gameplay

The system should not principally amount to meeting an invisible buildings-per-population ratio.

### DIRECTION — Participation and institutions

Temples, shrines, priests, festivals, processions, offerings, household worship, temple wealth/influence and regional cult importance are all valid directions.

The exact theological/historical abstraction and gameplay effects are **TBD**.

---

## Monuments

### DECIDED — Monument construction is central

Major monuments are one of the core fantasies of Egypt.

### REJECTED — Instant monument purchase

A monument must not simply appear when enough money/resources are deducted.

### DECIDED — Construction is physical and staged

Material should arrive at the site, labour and specialists should work, and the structure should visibly progress.

Desired phases can include survey/site preparation, foundations, material staging, ramps/scaffolding, structural courses, casing/finishing and decoration/dedication.

Different projects may require different materials, guilds and specialist labour.

### DECIDED — Monument labour competes with the rest of the city

Large projects should consume transport and labour that could otherwise serve agriculture, industry or other civic needs.

---

## Campaign / world

### DIRECTION — Dynasty/civilisation framing

The game should support the feeling of developing settlements across generations rather than playing disconnected puzzle maps.

### DIRECTION — Earlier settlements can remain relevant

A previous city may later act as a supplier, trading partner, religious centre, port, quarry settlement or strategic asset.

How much of an earlier city remains actively simulated versus abstracted is **TBD**.

### DECIDED — Settlements need distinct purposes

Maps/scenarios should not differ only by victory thresholds.

Examples include agricultural centres, ports, quarry/mining towns, royal capitals, temple cities, necropolises, frontier settlements and trading hubs.

---

## Military

### DIRECTION — Military may be part of the wider game

Pharaoh included armies, forts, walls, towers and naval warfare, so the reference system is documented.

Egypt's final military model is **TBD**.

### DIRECTION — City builder first

If combat is included, it should support the settlement simulation rather than transform the game into a conventional RTS.

---

## Architecture and technology

### DECIDED — Simulation should be separable from rendering

This is necessary for testing, performance work and future platform/rendering changes.

### DIRECTION — Modern scalability

Architecture should be chosen with large cities, many agents and modern multicore hardware in mind.

### TBD

No decision has yet been made on:

- programming language;
- commercial/open-source game engine vs custom engine;
- 2D/isometric/2.5D/3D rendering;
- ECS/data-oriented architecture;
- threading model;
- map dimensions/coordinate representation;
- save format;
- data/modding format;
- graphics API;
- asset pipeline;
- target platforms beyond desktop as the initial assumption.

These must remain open until deliberately decided.

---

## Initial playable proof

### DECIDED — First end-to-end chain

The foundational vertical slice is:

**place roads → establish housing → place clay pit → produce clay → physically move clay → make pottery → store/market pottery → physically distribute pottery → house recognises supply and improves**

### DECIDED — Follow-up chains

After pottery, add:

**reeds → papyrus maker → papyrus**

and a basic food/granary/market chain.

### DECIDED — Diagnosis is part of the milestone

The slice is not complete merely because the chain works. The player must also be able to inspect it and determine why it is broken when any stage is interrupted.

---

## Explicit non-goals / rejected inheritance

Unless deliberately reconsidered later, Egypt is **not** intended to:

- use original Pharaoh art/assets;
- copy original maps/scenarios/dialogue;
- require compatibility with Pharaoh save/data formats;
- inherit Akhenaten or any other reimplementation as its architecture by default;
- depend on random roamer pathing;
- use recruiter-touch labour logic;
- make rectangular road loops the only reliable residential layout;
- reduce religion to temple counts;
- make important goods teleport globally;
- hide supply failures behind unexplained status messages;
- make monuments instantaneous purchases.

---

## Open design questions

These remain intentionally unresolved:

1. What visual perspective best preserves Pharaoh's readability while looking like our own game?
2. How fine-grained should citizen simulation be?
3. How much traffic/congestion simulation is fun before it becomes transport micromanagement?
4. How dynamic should the Nile physically be?
5. How historically strict should the campaign timeline be?
6. How persistent should prior cities be?
7. How deep should social class and household economics become?
8. What level of military control belongs in the game?
9. Should maps be handcrafted, procedural or both?
10. What should be moddable/data-driven?
11. What is the appropriate maximum city/map scale?
12. Which engine/technology stack best supports the above without locking the project into unnecessary complexity?

These are **questions**, not accidental decisions.
