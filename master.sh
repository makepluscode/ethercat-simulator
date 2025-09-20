#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") [tui | test | --uds PATH | --tcp HOST:PORT] [--cycle us] [--debug] [--headless] [--no-auto]
Defaults: --uds /tmp/ethercat_bus.sock, --cycle 1000, headless auto-sequence enabled.
Use "tui" as the first argument to launch the interactive TUI instead.
Press ESC, Ctrl+C, or Ctrl+Z to exit gracefully.
Use "$(basename "$0") test" to run unit tests (Master/Slave scan scenarios).
USAGE
}

if [[ $# -gt 0 && "$1" == "test" ]]; then
  shift
  echo "[a-main.sh] Running unit tests (MainSlaveFixture)..."
  ./test.sh -R MainSlaveFixture "$@"
  exit $?
fi

ENDPOINT="uds:///tmp/ethercat_bus.sock"
CYCLE=1000
BUILD_DIR="${APP_BUILD_DIR:-build}"
DO_DEBUG=0
HEADLESS=1
AUTO=1
NO_ARGS=1

if [[ $# -gt 0 && "$1" == "tui" ]]; then
  HEADLESS=0
  NO_ARGS=0
  shift
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    --uds) ENDPOINT="uds://$2"; shift 2 ;;
    --tcp) ENDPOINT="tcp://$2"; shift 2 ;;
    --cycle) CYCLE="$2"; shift 2 ;;
    --debug) DO_DEBUG=1; shift ;;
    --headless) HEADLESS=1; shift ;;
    --no-auto) AUTO=0; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 2 ;;
  esac
  NO_ARGS=0
done

# Build binary if missing
BIN="${BUILD_DIR}/src/a-main/a-main"
if [[ ! -x "${BIN}" ]]; then
  echo "[build] ${BIN} not found. Building apps..."
  if [[ ${DO_DEBUG} -eq 1 ]]; then ./build.sh --debug; else ./build.sh; fi
fi

# Determine mode/arg from endpoint
MODE="--uds"; ARG="/tmp/ethercat_bus.sock"
if [[ "${ENDPOINT}" == uds://* ]]; then
  ARG="${ENDPOINT#uds://}"
elif [[ "${ENDPOINT}" == tcp://* ]]; then
  MODE="--tcp"; ARG="${ENDPOINT#tcp://}"
else
  echo "[error] Unsupported endpoint: ${ENDPOINT}" >&2; exit 2
fi

if [[ ${NO_ARGS} -eq 1 ]]; then
  echo "[a-main.sh] No arguments provided. Using defaults: ${MODE} ${ARG}, cycle=${CYCLE}"
fi
CMD=("${BIN}" "${MODE}" "${ARG}" "--cycle" "${CYCLE}")
if [[ ${HEADLESS} -eq 1 ]]; then
  CMD+=("--headless")
fi
if [[ ${AUTO} -eq 0 ]]; then
  CMD+=("--no-auto")
fi
echo "[a-main.sh] Running: ${CMD[*]}"
echo "[a-main.sh] Logging to a-main.log"
exec "${CMD[@]}" 2>&1 | tee a-main.log
