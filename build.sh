#!/usr/bin/env bash
set -euo pipefail

# Standardized build entrypoint for this repository.
# Examples:
#   ./build.sh               # Release build (Conan by default)
#   ./build.sh --debug       # Debug build
#   ./build.sh --clean       # Clean build directory then build

BUILD_DIR="build"
BUILD_TYPE="Release"
USE_CONAN=1
CLEAN=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --debug) BUILD_TYPE="Debug" ;;
    --release) BUILD_TYPE="Release" ;;
    --conan) USE_CONAN=1 ;;
    --clean) CLEAN=1 ;;
    -h|--help)
      cat <<EOF
Usage: $(basename "$0") [--debug|--release] [--clean]
Defaults: --release with Conan toolchain.
EOF
      exit 0
      ;;
    *) echo "Unknown option: $1" >&2; exit 2 ;;
  esac
  shift
done

if [[ ${CLEAN} -eq 1 ]]; then
  echo "[clean] Removing ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

cmake_common_flags=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")

if [[ ${USE_CONAN} -eq 1 ]]; then
  command -v conan >/dev/null 2>&1 || { echo "Conan is required for --conan"; exit 1; }
  # Use a repo-local Conan home to avoid writing outside the workspace
  export CONAN_HOME="${CONAN_HOME:-$(pwd)/.conan2}"
  echo "[conan] Detecting profile"
  conan profile detect || true
  echo "[conan] Installing dependencies for ${BUILD_TYPE}"
  # Enable required options so CMake can find dependencies (FastDDS optional)
  conan install . \
    -o ethercat-simulator*:with_fastdds=True \
    -o ethercat-simulator*:with_ftxui=True \
    -c tools.cmake.cmaketoolchain:generator="Unix Makefiles" \
    -of "${BUILD_DIR}" -s "build_type=${BUILD_TYPE}" --build=missing
  cmake_common_flags+=("-DCMAKE_TOOLCHAIN_FILE=${BUILD_DIR}/conan_toolchain.cmake")
fi

echo "[cmake] Configuring (${BUILD_TYPE})"
cmake -S . -B "${BUILD_DIR}" \
  -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF \
  -DBUILD_AMAIN=ON -DBUILD_ASUBS=ON \
  "${cmake_common_flags[@]}"

echo "[cmake] Building"
# Guard against stale nested TUI build directory conflicting with binary name
if [[ -d "${BUILD_DIR}/tui/tui" ]]; then
  echo "[warn] Removing stale directory that conflicts with TUI binary: ${BUILD_DIR}/tui/tui"
  rm -rf "${BUILD_DIR}/tui/tui"
fi
cmake --build "${BUILD_DIR}" -j --target a-main a-subs

echo "[done] Artifacts in ${BUILD_DIR} (apps in ${BUILD_DIR}/src)"
