#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") [test | --uds PATH | --tcp HOST:PORT] [--count N] [--debug]
Defaults: --uds /tmp/ethercat_bus.sock, --count 1
Press ESC, Ctrl+C, or Ctrl+Z to exit gracefully.
Use "$(basename "$0") test" to run unit tests (Master/Slave scan scenarios).
USAGE
}

if [[ $# -gt 0 && "$1" == "test" ]]; then
  shift
  echo "[a-slaves.sh] Running unit tests (MasterSlaveFixture)..."
  ./test.sh -R MasterSlaveFixture "$@"
  exit $?
fi

ENDPOINT="uds:///tmp/ethercat_bus.sock"
COUNT=1
BUILD_DIR="${APP_BUILD_DIR:-build}"
DO_DEBUG=0
NO_ARGS=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --uds) ENDPOINT="uds://$2"; shift 2 ;;
    --tcp) ENDPOINT="tcp://$2"; shift 2 ;;
    --count) COUNT="$2"; shift 2 ;;
    --debug) DO_DEBUG=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 2 ;;
  esac
  NO_ARGS=0
done

# Build binary if missing
BIN="${BUILD_DIR}/src/a-subs/a-subs"
if [[ ! -x "${BIN}" ]]; then
  echo "[build] ${BIN} not found. Building apps..."
  if [[ ${DO_DEBUG} -eq 1 ]]; then ./build.sh --debug; else ./build.sh; fi
fi

MODE="--uds"; ARG="/tmp/ethercat_bus.sock"
if [[ "${ENDPOINT}" == uds://* ]]; then
  ARG="${ENDPOINT#uds://}"
elif [[ "${ENDPOINT}" == tcp://* ]]; then
  MODE="--tcp"; ARG="${ENDPOINT#tcp://}"
else
  echo "[error] Unsupported endpoint: ${ENDPOINT}" >&2; exit 2
fi

if [[ ${NO_ARGS} -eq 1 ]]; then
  echo "[a-subs.sh] No arguments provided. Using defaults: ${MODE} ${ARG}, count=${COUNT}"
fi
echo "[a-subs.sh] Running: ${BIN} ${MODE} ${ARG} --count ${COUNT}"
echo "[a-subs.sh] Logging to a-subs.log"
exec "${BIN}" "${MODE}" "${ARG}" --count "${COUNT}" 2>&1 | tee a-subs.log
