#!/usr/bin/env bash
set -euo pipefail

# Run helper for EtherCATSimulator
# Usage:
#   ./run.sh            -> build if needed, run CLI(simul) and GUI both
#   ./run.sh cli        -> build if needed, run CLI only
#   ./run.sh gui        -> build if needed, run GUI only

MODE=${1:-all}
BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Release}

# Binaries
CLI_BIN="${BUILD_DIR}/examples/kickcat_scan/examples_kickcat_scan"
GUI_BIN="${BUILD_DIR}/gui/desktop/gui_desktop"

need_cli=false
need_gui=false
case "${MODE}" in
  all|ALL|both)
    need_cli=true; need_gui=true;;
  cli|CLI)
    need_cli=true;;
  gui|GUI)
    need_gui=true;;
  *)
    echo "[run.sh] Unknown mode: ${MODE}. Use: cli | gui | (default) all" >&2
    exit 2;;
esac

ensure_build() {
  local want_gui=$1
  local need_build=false
  if ${need_cli} && [[ ! -x "${CLI_BIN}" ]]; then need_build=true; fi
  if ${want_gui} && [[ ! -x "${GUI_BIN}" ]]; then need_build=true; fi
  if ! ${need_build}; then return 0; fi

  if ${want_gui}; then
    # GUI needs Qt via Conan and BUILD_GUI=ON
    if ! command -v conan >/dev/null 2>&1; then
      echo "[run.sh] GUI requested but Conan not found. Please install Conan or run in cli mode." >&2
      exit 3
    fi
    echo "[run.sh] Building with Qt6/QML support..."
    CONAN_OPTIONS="-o with_qt=True" EXTRA_CMAKE_FLAGS="-DBUILD_GUI=ON" BUILD_TYPE="${BUILD_TYPE}" ./build.sh
  else
    echo "[run.sh] Building (no GUI)..."
    BUILD_TYPE="${BUILD_TYPE}" ./build.sh
  fi
}

ensure_build ${need_gui}

rc=0
if ${need_cli}; then
  echo "[run.sh] Running CLI: ${CLI_BIN}"
  "${CLI_BIN}" ${SIM_ARGS:-} || rc=$?
fi

if ${need_gui}; then
  echo "[run.sh] Running GUI: ${GUI_BIN}"
  "${GUI_BIN}" ${GUI_ARGS:-} || rc=$?
fi

exit ${rc}

