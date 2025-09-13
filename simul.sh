#!/usr/bin/env bash
set -euo pipefail

# Build and run the 'simul' app (FastDDS publisher)
# Usage:
#   ./simul.sh           -> build with FastDDS (Conan) and run simul

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Release}

# Ensure dependencies + build
CONAN_OPTIONS="-o with_fastdds=True ${CONAN_OPTIONS:-}" \
BUILD_TYPE="${BUILD_TYPE}" \
./build.sh

BIN="${BUILD_DIR}/examples/simul/simul"
if [[ ! -x "$BIN" ]]; then
  # If not built via Conan toolchain, try to configure toolchain if present
  if [[ -f "${BUILD_DIR}/conan_toolchain.cmake" ]]; then
    cmake -S . -B "${BUILD_DIR}" -DCMAKE_TOOLCHAIN_FILE="${BUILD_DIR}/conan_toolchain.cmake"
    cmake --build "${BUILD_DIR}" -j
  fi
fi

echo "[simul.sh] Running: $BIN"
"$BIN" "$@"

