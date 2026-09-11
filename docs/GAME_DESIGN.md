# Egypt — Game Design Record

Status: **Living design document / pre-production**

This file records the design intent agreed so far so that the project does not depend on chat history or memory. Undecided matters are marked **TBD** rather than being silently invented.

## 1. Project identity

**Working title:** Egypt

**Genre:** Egyptian city builder / civilisation simulation.

**Primary inspiration:** *Pharaoh* (1999) and *Cleopatra: Queen of the Nile*.

**Intent:** Build an original game that captures the best qualities of classic Egyptian city builders while using a cleaner, deeper and more modern simulation. This is not intended to be a clone and must use original code, systems, content, art, balancing, scenarios, UI and identity.

## 2. Core fantasy

The player establishes and develops Egyptian settlements whose economy, population and physical form can be understood by watching the city operate.

The city is not merely a collection of stat-producing buildings. People work, goods move, services travel, the Nile floods, farms change with the seasons, neighbourhoods prosper or decline and monuments visibly rise from accumulated labour and material.

The player should feel that they are governing a living city rather than operating a spreadsheet with Egyptian graphics.

## 3. Governing design principle

> **If something important happens in the simulation, wherever practical the player should be able to see it happening in the city.**

Examples:

- reeds are gathered and carried to a papyrus workshop;
- clay leaves a clay pit and reaches a potter;
- finished pottery is transported to storage or a market;
- market buyers obtain goods and traders distribute them to homes;
- floodwater visibly rises and retreats;
- fields visibly change through flooding, sowing and harvest;
- labour moves between seasonal agricultural and construction demands;
- quarried stone is moved across the map and incorporated into monuments;
- neighbourhoods visibly improve or deteriorate with living standards.

## 4. Core gameplay loop

The intended high-level loop is:

**land → roads/infrastructure → housing → food → workers → raw materials → industry → logistics → markets/services → improved housing/population → tax/trade wealth → major civic and monumental projects → larger and more complicated city**

Growth should create new opportunities and new logistical problems rather than simply larger numbers.

## 5. World and terrain

The map should support at least:

- Nile / permanent water;
- floodplain;
- fertile agricultural land;
- desert;
- rocky/quarry terrain;
- reeds / wetland resource areas;
- roads and crossings;
- irrigation infrastructure;
- wildlife and ambient life.

The Nile is a simulation system, not scenery.

### Nile cycle

Target behaviour:

- water level changes visibly over time;
- flood extent can vary;
- inundation affects floodplain fertility;
- farming operates around the agricultural cycle;
- irrigation can extend or stabilise agricultural potential;
- forecasts may become possible through suitable institutions/technology;
- seasonal labour demand changes when fields cannot be worked;
- river access and temporary water conditions can influence transport.

The exact historical calendar model and level of abstraction are **TBD**.

## 6. Population and housing

Residential land begins modestly and evolves according to actual access to goods, food, services, employment and neighbourhood conditions.

The player should not simply place an elite mansion directly. Better housing should emerge because the city successfully supports a higher standard of living.

Possible needs include:

- water;
- one or more food types;
- religion;
- pottery;
- beer;
- linen;
- healthcare;
- education;
- entertainment;
- administration;
- luxury goods;
- desirability / environment;
- employment and household prosperity.

The final progression ladder and exact requirements are **TBD** and should be original rather than copied numerically from *Pharaoh*.

Housing may decline if its requirements cease to be met.

## 7. Resources and production

Production chains are a major part of the game and should interconnect rather than exist as isolated one-building recipes.

Initial/high-priority examples:

- **Reeds → Papyrus workshop → Papyrus**
- **Clay → Potter → Pottery**
- **Clay + Straw → Bricks**
- **Grain → Food storage / consumption**
- **Grain → Straw by-product**
- **Barley → Brewery → Beer**
- **Flax → Weaver → Linen**
- **Stone quarry → storage/transport → monuments/building**

Later chains can include timber, copper, weapons, luxury goods, paints, lamps, grave goods and imported materials.

By-products and shared inputs are encouraged where they create understandable strategic choices.

## 8. Logistics

Goods should normally have physical location and physical movement.

A resource should not teleport between arbitrary buildings merely because both exist in the city.

Logistics actors may include:

- producer delivery workers;
- carts/donkeys;
- warehouse workers;
- market buyers;
- market distributors;
- boats and river freight;
- specialised monument transport teams.

Storage policy, destination selection, priority rules and transport capacity should become meaningful city-planning tools.

The simulation should scale without requiring every distant or irrelevant entity to update at full frequency every frame.

## 9. Roads, movement and service walkers

Walkers are important because they make the city readable and alive. We deliberately do **not** want to reproduce the classic random-roamer weakness where efficient cities require artificial rectangular road loops.

Service agents should make purposeful decisions using information relevant to their role.

Examples:

- a physician should favour homes that are overdue for service;
- a fire patrol should favour areas with greater current fire risk;
- a market distributor should favour homes missing goods carried by that market;
- a buyer should choose sensible suppliers based on availability, travel cost and policy;
- delivery workers should choose viable storage/manufacturing destinations.

Road layout should still matter through distance, congestion, crossings, accessibility and district design.

Potential player controls include:

- service districts;
- gates;
- road restrictions;
- preferred freight routes;
- building service radii or priorities.

These are design tools, not mandatory exploits.

## 10. Employment

The old model in which a recruiter merely needs to touch nearby housing should not be copied.

Workers should have a believable relationship to where they live and work.

Target concepts:

- neighbourhood labour pools;
- commuting distance/time;
- transport access;
- job desirability and/or skill where useful;
- seasonal labour demand;
- remote work camps/settlements for mines, quarries and major projects.

A remote quarry should need a genuine labour solution because people must reach it, not because a magic recruiter needs to touch a hut.

The exact population-agent granularity is **TBD**: individual citizens, household-level simulation, cohorts or a hybrid may be chosen for scale.

## 11. Markets and distribution

Markets should make household supply visible.

Target flow:

**producer/import → storage/granary → market buyer → market inventory → distributor → houses**

Market behaviour should be inspectable. A player should be able to determine why a neighbourhood lacks a good and trace the problem backwards through the chain.

## 12. Diagnostics and overlays

The game should be exceptionally good at explaining its own simulation.

Selecting a house should be able to answer questions such as:

- Why will this house not upgrade?
- What goods are missing?
- When was it last serviced?
- What is its food diversity?
- Is employment adequate?
- What is local desirability?
- What service is failing?

For a missing resource, the UI should be able to expose a causal chain such as:

**House ← Market ← Warehouse ← Pottery workshop ← Clay pit**

and highlight the bottleneck.

Desired overlays eventually include:

- fire risk;
- structural risk;
- health;
- water;
- market coverage;
- individual goods;
- employment/commute;
- desirability;
- religion;
- education;
- entertainment;
- taxation;
- congestion;
- freight routes;
- flood/fertility/irrigation.

## 13. Religion

Religion should remain important but should not reduce to spamming a required number of temples to satisfy an invisible ratio.

Potential systems:

- temples and shrines;
- priests;
- household worship;
- festivals and processions;
- offerings;
- temple wealth and influence;
- local/regional cult importance;
- gods whose domains interact with relevant parts of the simulation.

Historically inspired Egyptian religion can shape events and civic life, while exact gameplay abstractions remain **TBD**.

## 14. Monuments

Monument construction is one of the central fantasies of the project.

A monument must be a **physical project**, not an instant purchase or simple progress bar.

Possible construction phases:

1. site selection/survey;
2. ground preparation;
3. foundations;
4. material accumulation;
5. ramps/scaffolding/work areas where appropriate;
6. staged structural construction;
7. casing/finishing;
8. decoration or dedication.

Material should visibly arrive. Workers and specialist crews should visibly work. The structure should visibly change as construction progresses.

Different monuments may require different guilds/specialists, tools and material chains.

Major projects should compete with agriculture, commerce and other city needs for labour and transport capacity.

## 15. Trade

Trade should connect the city to a wider Egyptian and regional economy.

Potential features:

- land and river trade routes;
- imports and exports;
- trading partners;
- route capacity;
- transport time;
- prices influenced by supply/demand or scenario conditions;
- goods unobtainable locally;
- strategic reliance on earlier settlements in campaign play.

Exact economic modelling is **TBD**.

## 16. Persistent dynasty / campaign world

The campaign should not necessarily discard every previous city when a scenario ends.

Long-term target: the player builds a dynasty/civilisation in which previous settlements can remain relevant.

Examples:

- an earlier agricultural city becomes a later food supplier;
- a quarry settlement supplies a future capital;
- a temple founded decades earlier gains religious/political influence;
- a canal or infrastructure project changes future possibilities;
- previous success or failure affects trade, prestige or available resources.

Scenarios should have a reason to exist rather than merely asking for another generic city.

Possible settlement identities:

- floodplain agricultural centre;
- quarry/mining settlement;
- royal capital;
- river port;
- frontier town;
- temple city;
- necropolis;
- military/logistics centre;
- trading hub.

## 17. Civic systems planned for later development

The broader game may include:

- taxation and treasury;
- administration;
- healthcare;
- sanitation/water;
- education and scribes;
- entertainment;
- crime and policing;
- fire;
- structural collapse;
- disease;
- military;
- diplomacy;
- disasters/events;
- migration;
- social classes;
- river navigation;
- funerary economy;
- tomb robbery;
- prestige and royal authority.

These should be added only when the core simulation remains understandable and performant.

## 18. Wildlife and atmosphere

The map should feel alive independently of explicit economic actors.

Target wildlife includes historically/contextually appropriate animals such as crocodiles, hippopotamuses and birds.

A project requirement established at conception: **bin-chicken-like ibis are staying.**

Ambient activity should support the living-city feel without overwhelming simulation cost.

## 19. Simulation architecture principle

The simulation should be decoupled from presentation wherever practical.

Conceptual structure:

```text
WORLD
 ├── Terrain / Nile / seasons
 ├── Road and transport network
 ├── Buildings
 │    ├── Residential
 │    ├── Resource extraction
 │    ├── Manufacturing
 │    ├── Storage
 │    ├── Markets
 │    ├── Services
 │    └── Civic / religious / monumental
 ├── Population / households / labour
 ├── Walkers and transport agents
 ├── Resources / inventories
 ├── Economy / trade
 ├── Religion / culture
 ├── Events
 └── Diagnostics
        ↓
   Simulation ticks
        ↓
   Renderer / animation / UI
```

The renderer should show simulation state, not be the authoritative source of it.

The simulation should be designed for modern multicore hardware and large cities, but the exact threading/data-oriented architecture is **TBD** until the technology stack is selected.

## 20. First playable milestone

The first milestone is intentionally narrow:

1. Display a map with desert, Nile/floodplain and resource terrain.
2. Place and remove roads.
3. Zone/place basic housing.
4. Population can arrive and occupy housing.
5. Place a clay pit.
6. Clay is produced.
7. A physical logistics agent moves clay to a pottery workshop.
8. Pottery is produced.
9. Pottery can enter storage/market inventory.
10. A market distributor physically delivers pottery to houses.
11. Houses recognise the delivery and visibly improve when requirements are met.
12. The player can inspect each stage and identify a broken link.

Then add the second representative chain:

**reeds → papyrus workshop → papyrus**

and a basic food/granary chain.

If this slice is fun, visible, diagnosable and performant, it proves most of the foundational architecture.

## 21. Things we explicitly want to improve over Pharaoh

- No dependence on random service-roamer intersection choices.
- No requirement to design every residential area as a closed rectangular road loop.
- No fake labour recruiter touching a house while workers teleport conceptually from elsewhere.
- Better diagnosis of why a system is failing.
- More physically convincing logistics and commuting.
- More meaningful religion than temple-count ratios.
- Richer seasonal Nile behaviour.
- Monument building with more visible phases and logistics.
- Campaign settlements that can have persistent consequences.
- Modern performance and scalability.
- Large cities should remain understandable rather than merely becoming opaque.

## 22. Things we explicitly want to preserve in spirit

- Immediate, readable city-building.
- Roads full of visible activity.
- Interconnected production chains.
- Evolving housing.
- Bazaars/markets distributing real goods.
- Nile-centred agriculture.
- Egyptian atmosphere and historical progression.
- Trade and imports/exports.
- Gods, temples and festivals.
- Monument construction as a major goal.
- Scenarios with distinct economic/geographic identities.
- The satisfaction of watching a functioning city.

## 23. Decisions not yet made

The following remain **TBD**:

- final game title;
- programming language;
- game engine vs custom engine;
- 2D/isometric/2.5D/3D presentation;
- camera model;
- art style;
- animation pipeline;
- exact citizen simulation granularity;
- map size and coordinate model;
- save format;
- modding/data format;
- multiplayer (if any);
- target operating systems beyond an assumed desktop focus;
- exact historical period range;
- campaign structure details;
- exact economic formulae;
- combat design;
- final list of resources/buildings/gods/services.

These should be decided deliberately rather than accidentally becoming architecture constraints.

## 24. Design guardrail

Whenever a new system is proposed, ask:

1. Can the player see or understand what it is doing?
2. Does it interact with the city rather than existing as an isolated meter?
3. Does it create an interesting planning decision?
4. Can the player diagnose failure without guessing?
5. Does it make the city feel more alive?
6. Can it scale to a large settlement?

If the answer to most of these is no, the system needs redesign before implementation.
