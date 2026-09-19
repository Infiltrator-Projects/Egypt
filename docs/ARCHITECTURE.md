# Architecture

## Purpose

Egypt is an original Egyptian city-building and civilisation simulation built around visible causal systems rather than a compatibility clone of Pharaoh.

## System decomposition

- C++ game core
- custom CPU framebuffer/renderer
- native X11 platform layer
- world/simulation state
- screen/input state
- asset-driven presentation
- world and compile smoke tests
- design/research documentation

## Ownership boundaries

Historical research and older city builders are evidence and inspiration. Egypt owns its engine, simulation rules, balance, art and identity. Common historical or genre conventions survive only when they serve this simulation.

Platform APIs, hosted services, research sources and first-party shared libraries provide mechanisms or evidence behind explicit boundaries. They do not silently own the product's interpretation or policy.

## Source of truth

Executable behaviour is defined by code and tests. This document defines architectural ownership and dependency direction. Specialist documents refine narrower domains and must remain consistent with it.

## Change discipline

Keep platform handles/toolkit details out of domain contracts where practical. Keep generic behaviour in its shared owner rather than copying it. Preserve explicit unavailable/unsupported/failure states across layers.

## Specialist documentation

- docs/GAME_DESIGN.md
- docs/DESIGN_DECISIONS.md
- docs/TECHNICAL_DECISIONS.md
- docs/PHARAOH_REFERENCE.md
- docs/PHARAOH_FORENSIC_REFERENCE.md
- docs/GRAPHICAL_SCREEN_ARCHITECTURE.md
- docs/WILDLIFE_SIMULATION.md
