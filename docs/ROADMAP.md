# Egypt — Development Roadmap

Status: **Pre-production roadmap**

This roadmap is intentionally milestone-based rather than date-based. It is meant to stop the project from trying to build the entire final game at once.

A milestone is complete when its systems work together as a playable whole and can be diagnosed when they fail.

---

## Phase 0 — Foundation decisions

Goal: choose the minimum technology and simulation foundations required to begin without prematurely deciding the entire final architecture.

Decisions to make:

- language;
- engine/custom engine decision;
- rendering perspective;
- basic map representation;
- simulation tick model;
- save/test strategy;
- initial data format for resources/buildings;
- supported development platform(s).

Deliverables:

- project builds cleanly;
- automated tests can run without graphics;
- a blank map can be loaded;
- simulation can advance deterministically enough for repeatable tests;
- no unnecessary feature work yet.

---

## Phase 1 — World and placement prototype

Goal: make a map on which a city can physically exist.

Systems:

- terrain tiles/cells or equivalent;
- desert;
- Nile/permanent water;
- floodplain;
- resource terrain for clay/reeds;
- basic camera/input;
- road placement/removal;
- building placement/removal;
- occupancy/collision rules;
- basic inspection UI.

Success condition:

The player can load an Egyptian map, navigate it and place/remove roads and buildings with clear feedback.

---

## Phase 2 — Population and housing seed

Goal: establish the smallest believable residential loop.

Systems:

- residential zoning/placement;
- immigration;
- occupied houses;
- basic population accounting;
- road access requirement;
- one or two basic household needs;
- visible housing state change;
- basic local desirability placeholder if required by architecture.

Success condition:

Housing fills with population for understandable reasons and can visibly improve or decline when a requirement changes.

---

## Phase 3 — First complete production chain

Goal: prove the central simulation architecture end-to-end.

Required chain:

**Clay Pit → clay transport → Potter → pottery transport/storage → Market → household pottery delivery → housing response**

Systems:

- raw resource production;
- building inventories;
- goods with physical location;
- transport agent;
- destination selection;
- workshop input/output;
- storage;
- market buying;
- market distribution;
- household inventory/need;
- visible agent movement;
- failure states.

Success condition:

The player can watch one unit/batch of clay become pottery and eventually reach a house. Breaking any link causes a visible, explainable shortage.

This is the first major architectural proof.

---

## Phase 4 — Diagnostics proof

Goal: prove that complexity remains understandable.

Required capabilities:

- click a house and see why it cannot improve;
- inspect market stock;
- inspect storage stock/policies;
- inspect workshop input/output blockage;
- inspect raw-material supply;
- trace pottery shortage backward through the chain;
- highlight relevant buildings/routes;
- distinguish 'not produced', 'not transported', 'not stored', 'market cannot obtain', and 'distribution cannot reach'.

Success condition:

A new tester can intentionally break the pottery chain and identify the cause from the game UI rather than from debug logs.

---

## Phase 5 — Second and third production chains

Goal: demonstrate that the architecture generalises instead of being hard-coded around pottery.

Add:

**Reeds → Papyrus Maker → Papyrus**

and:

**Farm/Food source → Granary → Market → House**

Then add a by-product/shared-input example:

**Grain → food + straw**

with straw later available to another chain.

Success condition:

New resources/buildings are primarily data/configuration plus reusable system logic, not copy-pasted bespoke code.

---

## Phase 6 — Nile and agricultural cycle

Goal: make Egypt mechanically Egyptian.

Systems:

- seasonal calendar;
- flood state;
- visible flood rise/retreat;
- floodplain fertility;
- sow/grow/harvest cycle;
- irrigation concept;
- food storage across seasons;
- agricultural labour requirements;
- early flood forecast/Nilometer concept.

Success condition:

The city must plan around the river cycle, and the player can visually understand why a harvest is strong, weak, early, late or impossible.

---

## Phase 7 — Labour and commuting

Goal: replace the classic recruiter abstraction with our own believable employment model.

Systems:

- labour demand by building;
- labour availability by population/household/cohort;
- commuting/accessibility;
- job assignment;
- remote industry constraints;
- seasonal labour reallocation;
- work-camp/temporary labour concept where appropriate;
- labour diagnostic overlay.

Success condition:

A remote quarry or farm fails for a physically understandable labour-access reason, and the player can solve it with settlement/transport/planning rather than an arbitrary recruiter trick.

---

## Phase 8 — Purposeful service agents

Goal: prove that visible walkers can remain without Pharaoh's random-roamer limitation.

Candidate services:

- water delivery;
- health;
- fire prevention/response;
- religion;
- market distribution.

Systems:

- target scoring/prioritisation;
- route planning;
- service capacity;
- service expiry/need;
- district/routing controls if required;
- road congestion foundations if useful.

Success condition:

Organic street layouts remain viable, while road distance and topology still matter.

---

## Phase 9 — Neighbourhood progression

Goal: turn the functioning economy into visible urban development.

Systems:

- original housing progression ladder;
- multiple food types;
- pottery/beer/linen or equivalent goods;
- water/service tiers;
- desirability/environment;
- education/health/religion/entertainment hooks;
- household prosperity;
- housing decline/recovery;
- early social-class effects.

Success condition:

Two neighbourhoods supplied by the same city can develop differently for understandable spatial/economic reasons.

---

## Phase 10 — Monument construction prototype

Goal: prove the second central fantasy of the project.

Start with one manageable monument rather than a Great Pyramid.

Systems:

- construction site;
- staged build states;
- material requirements;
- material stockpiles;
- hauling crews;
- specialist workers/guilds;
- temporary construction works;
- visible structural progress;
- construction diagnostics.

Success condition:

The player can watch material leave the economy, reach the site and physically become part of a growing monument over time.

---

## Phase 11 — Full civic city loop

Goal: turn the prototype into a recognisable complete city builder.

Add/expand:

- taxation/treasury;
- health;
- education;
- entertainment;
- religion;
- crime/policing;
- fire;
- structural risk;
- administration;
- broader industry;
- trade;
- richer storage controls;
- transport improvements;
- city sentiment/migration.

Success condition:

A medium-sized city can survive, prosper, fail and recover through interacting systems.

---

## Phase 12 — Trade and regional economy

Goal: make individual settlements part of a wider world.

Systems:

- regional map;
- trade partners;
- imports/exports;
- route capacity/time;
- local resource scarcity;
- requests/obligations;
- city specialisation;
- earlier-settlement relationships where practical.

Success condition:

At least one settlement depends on trade for a critical input and another can profit by specialising in export production.

---

## Phase 13 — Dynasty/campaign persistence prototype

Goal: prove our major campaign departure from Pharaoh.

Systems:

- player dynasty/family continuity;
- historical progression;
- persistent settlement records;
- prior city output/relationships represented in later play;
- scenario-specific purpose;
- consequences carried forward.

Success condition:

Something built or decided in an earlier settlement materially changes a later settlement.

---

## Phase 14 — Military / conflict prototype if retained

Goal: determine the appropriate role of conflict without compromising the city-builder identity.

Possible systems:

- defensive works;
- garrisons;
- supply of weapons/chariots;
- regional military requests;
- limited battlefield control or abstract deployment;
- naval defence/transport.

This phase is conditional. The exact military model remains **TBD**.

---

## Phase 15 — Content expansion

Once the systems are stable, expand content rather than inventing new architecture unnecessarily.

Examples:

- more crops;
- more industries;
- more household goods;
- more gods/cults/institutions;
- more monuments;
- more settlement types;
- more trade goods;
- more wildlife;
- funerary economy;
- tombs and grave robbery;
- specialist infrastructure;
- historical events/scenarios.

---

## Phase 16 — Scale, performance and polish

Performance work occurs throughout development, but this phase validates final ambitions.

Targets to establish later:

- city population scale;
- agent counts;
- freight counts;
- map dimensions;
- save/load time;
- simulation speed multipliers;
- frame-time budget;
- memory budget;
- pathfinding budget;
- deterministic/reproducible simulation test cases.

Also includes:

- UI polish;
- accessibility;
- audio;
- animation;
- visual atmosphere;
- tutorial/onboarding;
- scenario editor/modding if retained;
- robust save migration policy once releases require it.

---

## Immediate first objective

Before broad content work, prove this exact experience:

> A player places a road and homes, opens a clay pit and potter, watches clay physically travel to the potter, watches pottery reach a market, sees a seller deliver it to a house, watches that house improve, then can break any link and use the UI to discover exactly what failed.

If that is satisfying, legible and technically clean, the foundation of Egypt is real.
