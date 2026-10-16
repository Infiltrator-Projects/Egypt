#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${1:-$root/build-package}"
dist_dir="${2:-$root/dist}"
version="$(tr -d '\r\n' < "$root/VERSION")"
arch="$(dpkg --print-architecture)"

[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || { echo "Unable to determine Egypt version" >&2; exit 1; }
command -v cmake >/dev/null
command -v dpkg-deb >/dev/null
command -v python3 >/dev/null

mkdir -p "$build_dir" "$dist_dir"
build_dir="$(cd "$build_dir" && pwd)"
dist_dir="$(cd "$dist_dir" && pwd)"
SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(git -C "$root" log -1 --format=%ct)}"
export SOURCE_DATE_EPOCH

cmake -S "$root" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DEGYPT_HOST_BACKEND=x11 \
    -DEGYPT_BUILD_PROFILE=generic
cmake --build "$build_dir" --parallel
ctest --test-dir "$build_dir" --output-on-failure

stage="$build_dir/package-root"
rm -rf "$stage"
DESTDIR="$stage" cmake --install "$build_dir" --component Runtime

mkdir -p "$stage/DEBIAN"
cat >"$stage/DEBIAN/control" <<EOF
Package: egypt
Version: $version
Section: games
Priority: optional
Architecture: $arch
Maintainer: Shannon Smith <noreply@github.com>
Depends: libc6, libgcc-s1, libstdc++6, libx11-6
Homepage: https://github.com/Infiltrator-Projects/Egypt
Description: native Egyptian city-building simulation
 Egypt is a native C++ Egyptian city-building and civilisation simulation.
 This package is the generic CPU build; hardware-native builds are produced
 directly from the same source tree with EGYPT_BUILD_PROFILE=native.
EOF

output="$dist_dir/egypt_${version}_${arch}.deb"
dpkg-deb --root-owner-group --build "$stage" "$output"
printf '%s\n' "$output"
