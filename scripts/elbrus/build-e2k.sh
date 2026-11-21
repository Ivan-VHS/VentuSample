#!/usr/bin/env bash
set -euo pipefail

export CXX="${E2K_CXX:-${CXX:-g++}}"
export CC="${E2K_CC:-${CC:-gcc}}"
BUILD_DIR="${BUILD_DIR:-build-elbrus}"

if ! command -v "$CXX" >/dev/null 2>&1; then
  echo "E2K C++ compiler not found: $CXX" >&2
  exit 2
fi
if ! printf '\n' | "$CXX" -dM -E -x c++ - 2>/dev/null | grep -Eq '^#define[[:space:]]+__e2k__([[:space:]]|$)'; then
  echo "$CXX does not define __e2k__; refusing to label a non-E2K build as native E2K." >&2
  exit 2
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure
