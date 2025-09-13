#!/usr/bin/env bash
set -euo pipefail

# Simple test runner for GoogleTest via CTest
# Usage: test.sh [--debug|--release] [--conan|--no-conan] [--fresh] [-j N] [-- <ctest-args>]
# Examples:
#   bash ./test.sh --debug --fresh
#   bash ./test.sh --release -j 8 -- -R Communication

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
USE_CONAN=${USE_CONAN:-auto}  # auto|on|off
FRESH=0

# Determine parallel jobs
if command -v nproc >/dev/null 2>&1; then
  JOBS_DEFAULT=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
  JOBS_DEFAULT=$(sysctl -n hw.ncpu)
else
  JOBS_DEFAULT=4
fi
JOBS=${JOBS:-$JOBS_DEFAULT}

CTEST_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --release) BUILD_TYPE=Release; shift ;;
    --debug)   BUILD_TYPE=Debug; shift ;;
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    --fresh)  FRESH=1; shift ;;
    --conan)   USE_CONAN=on; shift ;;
    --no-conan) USE_CONAN=off; shift ;;
    -j|--jobs) JOBS="$2"; shift 2 ;;
    --) shift; CTEST_ARGS+=("$@"); break ;;
    -h|--help)
      echo "Usage: $0 [--debug|--release] [--conan|--no-conan] [-j N] [-- <ctest-args>]";
      exit 0 ;;
    *) CTEST_ARGS+=("$1"); shift ;;
  esac
done

CMAKE_TOOLCHAIN_ARG=()

if [[ $FRESH -eq 1 ]]; then
  rm -rf "$BUILD_DIR"
fi

if [[ "$USE_CONAN" != "off" ]] && command -v conan >/dev/null 2>&1 && [[ -f conanfile.py ]]; then
  # Best-effort Conan setup; ignore profile detect failure if already present
  conan profile detect || true
  conan install . -of "$BUILD_DIR" -s build_type="$BUILD_TYPE" --build=missing
  CMAKE_TOOLCHAIN_ARG=("-DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake")
fi

cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DBUILD_EXAMPLES=OFF -DBUILD_GUI=OFF \
  -DFORCE_NO_FTXUI=ON -DFORCE_NO_FASTDDS=ON \
  ${CMAKE_TOOLCHAIN_ARG[@]:-}
cmake --build "$BUILD_DIR" -j "$JOBS"
ctest --test-dir "$BUILD_DIR" --output-on-failure "${CTEST_ARGS[@]}"
