# Egypt

**Project copyright:** © 2000–2026 Shannon Smith

**Egypt** is an original Egyptian city-building and civilisation simulation inspired by the design strengths of *Pharaoh* (1999) and *Cleopatra: Queen of the Nile*, but built as a new game with its own simulation, systems, art, balance, scenarios and identity.

The goal is not to clone *Pharaoh*. The goal is to preserve what made that style of city builder compelling — visible logistics, evolving neighbourhoods, Nile agriculture, interconnected industries, walkers, trade and monumental construction — while redesigning weak or artificial systems using modern simulation techniques.

## Engineering ethos

What happens when an Egyptian city builder is rebuilt from first principles, from the simulation outward, instead of treating a 1990s game's mechanics as a specification? Egypt begins with the causes the player should be able to see: water, land, labour, movement, production, distribution, services, housing and monumental work.

*Pharaoh*, *Cleopatra*, historical research and other city builders are design evidence. They can reveal ideas worth preserving and weaknesses worth correcting, but this project owns its simulation, engine, renderer, rules, balance, art and identity. A familiar mechanic survives because it still produces the strongest result, not because the older game happened to implement it that way.

"Modern" is not automatically "better". New simulation techniques are adopted when they improve causality, legibility, performance or player agency; proven older ideas remain when they still work better. Wherever practical, an important outcome should be traceable through the game's own visible systems rather than hidden behind an unexplained rule.

## Core vision

The player should be able to **watch the city work**.

> If something important happens in the simulation, wherever practical the player should be able to see it happening in the city.

Resources should move through real production chains. Workers and service providers should physically travel. Markets should collect and distribute goods. Houses should evolve because their residents actually receive food, services and manufactured goods. The Nile should shape the economy. Monument construction should be a long, visible civic undertaking rather than a progress bar.

## Current implementation

Egypt now has its first native executable shell.

- **Language:** C++20
- **Game engine:** our own
- **Renderer:** our own CPU framebuffer and drawing primitives
- **UI/font:** our own menu hit-testing and bitmap font renderer
- **Linux platform layer:** native X11 window/input/presentation path
- **First screen:** Egyptian-themed main menu
- **New Game:** enters a primitive Nile/floodplain/desert city screen

The current shell deliberately contains almost no simulation. Its purpose is to establish our executable, rendering, input and screen-state foundations before city systems are added.

On a Linux development machine with a C++20 compiler and X11 development headers available:

```text
make
./build/egypt
```

## Initial playable target

**Place road → establish housing → place clay pit → move clay → make pottery → store/distribute pottery → deliver pottery to houses → houses improve.**

The first slice should also include reeds and papyrus, food, basic housing, workers, storage, markets, a simple economy and an Egyptian map with Nile/floodplain/desert terrain.

The slice is not considered complete unless the player can also break any link in that chain and determine from the game UI exactly why it failed.

## Documentation

The repository documentation is deliberately split by purpose so later development does not depend on chat history or accidentally turn a proposal into a project requirement.

- [`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md) — living game-design vision.
- [`docs/DESIGN_DECISIONS.md`](docs/DESIGN_DECISIONS.md) — authoritative decision register.
- [`docs/PHARAOH_REFERENCE.md`](docs/PHARAOH_REFERENCE.md) — detailed *Pharaoh*/*Cleopatra* reference bible.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — staged development roadmap.
- [`docs/TECHNICAL_DECISIONS.md`](docs/TECHNICAL_DECISIONS.md) — implementation decisions and rejected directions.
- [`docs/BOOTSTRAP.md`](docs/BOOTSTRAP.md) — current native executable milestone.

## Still deliberately TBD

- final game title;
- final 2D/isometric/2.5D/3D presentation;
- exact citizen simulation granularity;
- final map scale;
- save/modding formats;
- final combat design;
- complete resource/building/content catalogue;
- Windows platform layer details.

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
