# Egypt

**Egypt** is an original Egyptian city-building and civilisation simulation inspired by the design strengths of *Pharaoh* (1999) and *Cleopatra: Queen of the Nile*, but built as a new game with its own simulation, systems, art, balance, scenarios and identity.

The goal is not to clone *Pharaoh*. The goal is to preserve what made that style of city builder compelling — visible logistics, evolving neighbourhoods, Nile agriculture, interconnected industries, walkers, trade and monumental construction — while redesigning weak or artificial systems using modern simulation techniques.

## Core vision

The player should be able to **watch the city work**.

> If something important happens in the simulation, wherever practical the player should be able to see it happening in the city.

Resources should move through real production chains. Workers and service providers should physically travel. Markets should collect and distribute goods. Houses should evolve because their residents actually receive food, services and manufactured goods. The Nile should shape the economy. Monument construction should be a long, visible civic undertaking rather than a progress bar.

## Initial playable target

**Place road → establish housing → place clay pit → move clay → make pottery → store/distribute pottery → deliver pottery to houses → houses improve.**

The first slice should also include reeds and papyrus, food, basic housing, workers, storage, markets, a simple economy and an Egyptian map with Nile/floodplain/desert terrain.

The slice is not considered complete unless the player can also break any link in that chain and determine from the game UI exactly why it failed.

## Documentation

The repository documentation is deliberately split by purpose so later development does not depend on chat history or accidentally turn a proposal into a project requirement.

- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — living game-design vision: what Egypt is intended to become and how its major systems should interact.
- [`docs/DESIGN_DECISIONS.md`](docs/DESIGN_DECISIONS.md) — authoritative decision register distinguishing **DECIDED**, **DIRECTION**, **PROPOSED**, **TBD** and **REJECTED** ideas.
- [`docs/PHARAOH_REFERENCE.md`](docs/PHARAOH_REFERENCE.md) — detailed reference bible for *Pharaoh*/*Cleopatra*: housing, Nile, farming, industry, markets, walkers, labour, civic systems, religion, monuments, military, campaign, UI, strengths, weaknesses and lessons for Egypt.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — staged development path from technology selection through the first pottery vertical slice, Nile agriculture, labour, service agents, monuments, trade and campaign persistence.

## Design status

The broad simulation philosophy is established, but a number of foundational technical choices remain deliberately **TBD**, including:

- final game title;
- programming language;
- engine vs custom engine;
- 2D/isometric/2.5D/3D presentation;
- exact citizen simulation granularity;
- final map scale;
- save/modding formats;
- final combat design;
- complete resource/building/content catalogue.

Those choices should be made deliberately rather than becoming accidental architecture constraints.

## Foundational principles already established

- The Nile is a simulation system, not scenery.
- Goods normally have physical location and must be transported.
- Visible walkers/agents are important, but random-roamer behaviour is not the basis of service coverage.
- Employment must have a believable relationship to where people live and how they reach work.
- Housing evolves from actual city conditions rather than direct upgrade buttons.
- Production chains should interconnect.
- The player should be able to diagnose supply and service failures causally.
- Monument construction is physical, staged and labour/material intensive.
- Earlier settlements should be able to matter in the wider dynasty/campaign where practical.
- Egypt is an original game, not a Pharaoh compatibility project.
- Ibis — the Egyptian bin chickens — are staying.
