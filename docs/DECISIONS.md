# Decisions

This file is the canonical cross-project decision index for Egypt. Deeper game-design decisions remain in `DESIGN_DECISIONS.md`; low-level implementation decisions remain in `TECHNICAL_DECISIONS.md`.

## ADR-001 — Egypt is an original simulation, not Pharaoh compatibility

**Decision.** Preserve useful genre ideas but own simulation rules, engine, presentation, balance and content.

**Rationale.** Compatibility would constrain the project to old mechanics even where a stronger causal model is possible.

**Consequence.** Pharaoh/Cleopatra research is evidence/reference, not a behavioural specification.

## ADR-002 — Important causes should be visible

**Decision.** Where practical, important simulation outcomes should be traceable through visible movement, production, service or environmental state.

**Rationale.** A city builder is more legible and satisfying when the player can understand why something succeeded or failed.

**Consequence.** Hidden dice/coverage rules are avoided when an inspectable causal system can serve the design.

## ADR-003 — Goods have location and move through chains

**Decision.** Resources normally exist somewhere and require transport through production/distribution.

**Rationale.** Physical logistics is central to the intended city simulation.

**Consequence.** Storage, markets, workers and roads are functional systems rather than decorative abstractions.

## ADR-004 — The Nile is a simulation system

**Decision.** Nile/floodplain/desert conditions participate in economy and settlement behaviour rather than acting as background art.

**Rationale.** Egyptian city development is inseparable from water, floodplain and agricultural constraints.

**Consequence.** Terrain/environment state belongs in world simulation and must affect production/planning.

## ADR-005 — Own the core engine/rendering path

**Decision.** The current project uses its own C++20 game core, CPU framebuffer/drawing and native platform layer rather than adopting a large third-party engine.

**Rationale.** The project values control, understandability and first-principles implementation.

**Consequence.** Engine functionality is added deliberately as the game needs it, and external engines remain reference points rather than runtime foundations.

## ADR-006 — Broken chains must be diagnosable

**Decision.** A production/service chain is not complete unless the player can identify why it is failing.

**Rationale.** Causal simulation without causal presentation becomes opaque.

**Consequence.** UI/inspection support is part of simulation-feature completion, not optional polish.
