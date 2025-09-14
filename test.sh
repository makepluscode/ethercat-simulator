#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "[error] Build directory '${BUILD_DIR}' not found. Run ./build.sh first." >&2
  exit 1
fi

echo "[ctest] Running tests in ${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

