# Changelog

## 0.1.2

- Replace both Python artwork tools with a native C++20 asset builder; preserve decoded menu SHA-256 and authored HUD checksums.
- Eliminate the Python interpreter requirement from Egypt CMake, Makefile, packaging, and CI.
- Validate all four generated artwork files from native output before packaging.

## 0.1.1

- Use one version authority and publish exact CI-qualified package/source bytes.
- Remove duplicate generic CI builds and destructive build-directory resets.
- Separate package qualification from publication and central APT scheduling.

Earlier product history remains in Git.
