#!/usr/bin/env bash
set -euo pipefail

# Simple build helper for this repo.
# Usage:
#   ./build.sh           -> configure + build (Release)
#   ./build.sh test      -> configure + build + run tests

MODE=${1:-build}
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Release}
JOBS=${JOBS:-$(command -v nproc >/dev/null 2>&1 && nproc || echo 4)}

echo "[build.sh] mode=${MODE} type=${BUILD_TYPE} dir=${BUILD_DIR} jobs=${JOBS}"

mkdir -p "${BUILD_DIR}"

if command -v conan >/dev/null 2>&1; then
  echo "[build.sh] Conan detected. Installing dependencies..."
  conan profile detect || true
  conan install . -of "${BUILD_DIR}" -s build_type="${BUILD_TYPE}" --build=missing
  TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=${BUILD_DIR}/conan_toolchain.cmake"
else
  echo "[build.sh] Conan not found. Proceeding without it."
  TOOLCHAIN=""
fi

echo "[build.sh] Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ${TOOLCHAIN}

echo "[build.sh] Building..."
cmake --build "${BUILD_DIR}" -j "${JOBS}"

if [[ "${MODE}" == "test" ]]; then
  echo "[build.sh] Running tests..."
  ctest --test-dir "${BUILD_DIR}" --output-on-failure
fi

echo "[build.sh] Done."

