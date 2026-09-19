# Validation

## Evidence model

Compilation, deterministic tests, integration tests and human/physical-environment validation demonstrate different properties and must not be conflated.

## Automated gates

- .github/workflows/ci.yml

tests/ currently provides game-core compile coverage and world smoke tests; the test surface should grow with simulation systems so deterministic rules remain regression-testable.

## Manual/environment evidence

Visual quality, playability, pacing and whether causal information is legible to a player require human playtesting in addition to deterministic simulation tests.

Manual observations should record the environment and behaviour actually tested; they supplement rather than replace deterministic regression coverage.

## Release criterion

The exact revision intended for release must satisfy its required automated checks and must not document planned or unverified behaviour as complete.

## Regression rule

Reproducible defects should become permanent tests at the narrowest useful layer. As the product grows, validation should grow with the owned behaviour rather than becoming a separate afterthought.
