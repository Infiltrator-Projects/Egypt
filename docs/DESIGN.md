# Design

## First-principles position

Egypt starts from the behaviour it must own. Existing products, research, provider APIs and tools are studied as evidence, then accepted, changed or rejected according to the needs of this project.

## Goals

- make important economic and civic causes visible to the player
- model goods, labour, services and Nile systems as connected simulation rather than decorative counters
- preserve proven genre ideas while replacing artificial mechanics when a better causal model exists
- keep engine/rendering foundations understandable and first-party

## Non-goals

Egypt is not a Pharaoh compatibility project and historical plausibility does not require pretending uncertain evidence is known. Presentation technology is subordinate to simulation clarity.

## Dependency and language policy

Prefer first-party C/C++ implementation for native/core behaviour where suitable. Use platform-native services where they provide a stronger documented contract. A dependency or external source must not become an undocumented source of semantic truth.

## Failure and uncertainty

Unavailable, unsupported, uncertain and failed are distinct. Prefer visible uncertainty or refusal to guessed success. Persistent or destructive operations require explicit preconditions and post-verification appropriate to their risk.

## Decision quality

A change should improve correctness, safety, fidelity, performance, usability or maintainability and include a validation method. Newness alone is not a design argument.
