#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") [--uds PATH | --tcp HOST:PORT] [--count N] [--debug]
Defaults: --uds /tmp/ethercat_bus.sock, --count 1
Press ESC, Ctrl+C, or Ctrl+Z to exit gracefully.
USAGE
}

ENDPOINT="uds:///tmp/ethercat_bus.sock"
COUNT=1
BUILD_DIR="${APP_BUILD_DIR:-build}"
DO_DEBUG=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --uds) ENDPOINT="uds://$2"; shift 2 ;;
    --tcp) ENDPOINT="tcp://$2"; shift 2 ;;
    --count) COUNT="$2"; shift 2 ;;
    --debug) DO_DEBUG=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 2 ;;
  esac
done

# Build binary if missing
BIN="${BUILD_DIR}/src/a-slaves/a-slaves"
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

restore_tty() {
  if [[ -n "${_OLD_STTY:-}" ]]; then stty "${_OLD_STTY}" || true; fi
}

cleanup_socket() {
  # Only remove UDS path when it's a filesystem path and we created it
  if [[ "${MODE}" == "--uds" && "${ARG}" != @* && -S "${ARG}" ]]; then
    rm -f "${ARG}" || true
  fi
}

shutdown() {
  echo
  echo "[a-slaves.sh] Stopping..."
  if [[ -n "${PID:-}" ]] && kill -0 "${PID}" 2>/dev/null; then
    kill -TERM "${PID}" 2>/dev/null || true
    wait "${PID}" 2>/dev/null || true
  fi
  restore_tty
  cleanup_socket
  echo "[a-slaves.sh] Done."
  exit 0
}

trap shutdown INT TERM TSTP

echo "[a-slaves.sh] Running: ${BIN} ${MODE} ${ARG} --count ${COUNT}"
"${BIN}" "${MODE}" "${ARG}" --count "${COUNT}" &
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
  wait "${PID}"
fi

restore_tty
cleanup_socket
exit 0

