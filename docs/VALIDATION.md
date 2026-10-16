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

## Build and publication ownership

Root VERSION owns source and package identity. The packaging script configures, builds and tests the generic package; CI separately qualifies the native profile and window-system-free core. Packaging reuses a valid build directory without deleting an arbitrary caller-supplied path.

The successful CI run seals the Debian package, exact Git source archive and commit identity with SHA-256. Publication downloads that run's artifact and compares remote bytes before and after draft promotion. Public tags and assets are immutable; advancing VERSION is the release signal. Commit subjects carry no control-plane meaning.

The source build retrieves Common at the exact commit recorded in CMake. Initial configuration therefore needs network access unless that dependency is already materialised. Human playtesting remains separate from automated core/package qualification.
