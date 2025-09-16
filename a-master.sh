#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") [--uds PATH | --tcp HOST:PORT] [--cycle us] [--debug]
Defaults: --uds /tmp/ethercat_bus.sock, --cycle 1000
Press ESC, Ctrl+C, or Ctrl+Z to exit gracefully.
USAGE
}

ENDPOINT="uds:///tmp/ethercat_bus.sock"
CYCLE=1000
BUILD_DIR="${APP_BUILD_DIR:-build}"
DO_DEBUG=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --uds) ENDPOINT="uds://$2"; shift 2 ;;
    --tcp) ENDPOINT="tcp://$2"; shift 2 ;;
    --cycle) CYCLE="$2"; shift 2 ;;
    --debug) DO_DEBUG=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 2 ;;
  esac
done

# Build binary if missing
BIN="${BUILD_DIR}/src/a-master/a-master"
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

restore_tty() {
  if [[ -n "${_OLD_STTY:-}" ]]; then stty "${_OLD_STTY}" || true; fi
}

shutdown() {
  echo
  echo "[a-master.sh] Stopping..."
  if [[ -n "${PID:-}" ]] && kill -0 "${PID}" 2>/dev/null; then
    kill -TERM "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi
  restore_tty
  echo "[a-master.sh] Done."
  exit 0
}

trap shutdown INT TERM TSTP

echo "[a-master.sh] Running: ${BIN} ${MODE} ${ARG} --cycle ${CYCLE}"
"${BIN}" "${MODE}" "${ARG}" --cycle "${CYCLE}" &
PID=$!

# Non-blocking ESC watcher (TTY only)
if [[ -t 0 ]]; then
  _OLD_STTY=$(stty -g || true)
  stty -echo -icanon time 0 min 0 || true
  while kill -0 "${PID}" 2>/dev/null; do
    if read -r -s -n 1 -t 0.2 key; then
      if [[ "${key}" == $'\e' ]]; then shutdown; fi
    fi
  done
else
  # Non-tty: just wait for process
  wait "${PID}"
fi

restore_tty
exit 0

