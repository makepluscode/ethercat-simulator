#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build-tests"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "[setup] Creating test build in '${BUILD_DIR}'"
  command -v conan >/dev/null 2>&1 || { echo "Conan is required"; exit 1; }
  export CONAN_HOME="${CONAN_HOME:-$(pwd)/.conan2}"
  conan profile detect || true
  conan install . \
    -o ethercat-simulator*:with_fastdds=True \
    -o ethercat-simulator*:with_ftxui=True \
    -c tools.cmake.cmaketoolchain:generator="Unix Makefiles" \
    -of "${BUILD_DIR}" -s "build_type=Debug" --build=missing
  cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE=${BUILD_DIR}/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_EXAMPLES=ON -DBUILD_TESTING=ON -DBUILD_AMASTER=OFF -DBUILD_ASLAVES=OFF
  cmake --build "${BUILD_DIR}" -j
fi

GROUP=""
EXPR=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    -L|--label) GROUP="$2"; shift 2 ;;
    -R|--regex) EXPR="$2"; shift 2 ;;
    -h|--help)
      cat <<USAGE
Usage: $(basename "$0") [--label <label>] [--regex <gtest/ctest-regex>]

Examples:
  ./test.sh                         # all tests
  ./test.sh -L examples             # only examples/* tests
  ./test.sh --label core            # only core-labeled unit tests
  ./test.sh -L ui                   # only TUI smoke test
  ./test.sh -R "EL1258|VirtualSlave"  # regex filter
USAGE
      exit 0 ;;
    *) shift ;;
  esac
done

echo "[ctest] Running tests in ${BUILD_DIR}${GROUP:+ (label=${GROUP})}${EXPR:+ (regex=${EXPR})}"
CTEST_ARGS=( --test-dir "${BUILD_DIR}" --output-on-failure )
if [[ -n "${GROUP}" ]]; then
  # Workaround: some CTest versions mishandle multi-labels; provide curated regex
  if [[ "${GROUP}" == "slave" && -z "${EXPR}" ]]; then
    EXPR="EL1258|VirtualSlave"
  fi
  CTEST_ARGS+=( -L "${GROUP}" )
fi
if [[ -n "${EXPR}" ]]; then CTEST_ARGS+=( -R "${EXPR}" ); fi
ctest "${CTEST_ARGS[@]}"
