#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "[error] Build directory '${BUILD_DIR}' not found. Run ./build.sh first." >&2
  exit 1
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
if [[ -n "${GROUP}" ]]; then CTEST_ARGS+=( -L "${GROUP}" ); fi
if [[ -n "${EXPR}" ]]; then CTEST_ARGS+=( -R "${EXPR}" ); fi
ctest "${CTEST_ARGS[@]}"
