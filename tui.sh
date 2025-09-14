#!/usr/bin/env bash
set -euo pipefail

#!/usr/bin/env bash
set -euo pipefail

# Build and run ONLY the TUI app in tui/src/main.cpp.
# FastDDS 통신은 기본 활성화입니다.
#
# Usage:
#   bash ./tui.sh [--debug|--release] [--fresh] [--dds|--no-dds] [--run-pub]
#                  [--kickcat-ref <ref>] [-j N] [-- <tui-args>]

# Dedicated build dir to avoid interfering with other builds/coverage artifacts
BUILD_DIR=${BUILD_DIR:-build/tui}
BUILD_TYPE=${BUILD_TYPE:-Release}
JOBS=${JOBS:-$(command -v nproc >/dev/null 2>&1 && nproc || echo 4)}
DDS=1
RUN_PUB=0
FRESH=0
KICKCAT_REF_DEFAULT=${KICKCAT_REF_DEFAULT:-kickcat/v2.0-rc1}

TUI_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --release) BUILD_TYPE=Release; shift ;;
    --debug)   BUILD_TYPE=Debug; shift ;;
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    -j|--jobs) JOBS="$2"; shift 2 ;;
    --dds)     DDS=1; shift ;;
    --no-dds)  DDS=0; shift ;;
    --run-pub|--pub) RUN_PUB=1; shift ;;
    --fresh)   FRESH=1; shift ;;
    --kickcat-ref) export KICKCAT_REF="$2"; shift 2 ;;
    --) shift; TUI_ARGS+=("$@"); break ;;
    -h|--help)
      cat <<USAGE
Build and run the TUI (FTXUI) app only.

Options:
  --release | --debug           Build type (default: Release)
  --build-type <type>           Explicit CMAKE_BUILD_TYPE
  -j, --jobs <N>                Parallel build jobs (default: auto)
  --dds | --no-dds              Enable/disable FastDDS subscriber in TUI (default: enabled)
  --run-pub | --pub             Also build+run examples/simul (publisher) in background
  --fresh                       Remove build directory before configuring
  --kickcat-ref <ref>           Conan reference for KickCAT (default: ${KICKCAT_REF_DEFAULT})
  --                            Pass remaining args to the TUI binary

Examples:
  bash ./tui.sh --dds --run-pub
  bash ./tui.sh --debug -- --custom-arg
USAGE
      exit 0 ;;
    *) TUI_ARGS+=("$1"); shift ;;
  esac
done

if [[ $FRESH -eq 1 ]]; then
  rm -rf "$BUILD_DIR"
fi

# Make sure Conan deps exist (FTXUI + KickCAT; FastDDS optional)
if command -v conan >/dev/null 2>&1; then
  conan profile detect || true
  CONAN_OPTS=(
    -o ethercat-simulator*:with_kickcat=True
    -o ethercat-simulator*:with_ftxui=True
    -o ethercat-simulator*:with_fastdds=True
  )
  if [[ -z "${KICKCAT_REF:-}" ]]; then
    export KICKCAT_REF="$KICKCAT_REF_DEFAULT"
  fi
  conan install . -of "$BUILD_DIR" -s build_type="$BUILD_TYPE" --build=missing ${CONAN_OPTS[*]}
else
  echo "[tui.sh] WARNING: Conan not found. Assuming dependencies are available to CMake."
fi

# Configure CMake for TUI-only build
CMAKE_ARGS=(
  -S .
  -B "$BUILD_DIR"
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
  -DBUILD_TESTING=OFF
  -DBUILD_EXAMPLES=$([[ $RUN_PUB -eq 1 ]] && echo ON || echo OFF)
  -DBUILD_GUI=OFF
  -DENABLE_COVERAGE=OFF
  -DFORCE_NO_FTXUI=OFF
)
if [[ -f "$BUILD_DIR/conan_toolchain.cmake" ]]; then
  CMAKE_ARGS+=( -DCMAKE_TOOLCHAIN_FILE="$BUILD_DIR/conan_toolchain.cmake" )
  # Help CMake find CMakeDeps packages
  CMAKE_ARGS+=( -DCMAKE_PREFIX_PATH="$(pwd)/$BUILD_DIR" )
fi

echo "[tui.sh] Configuring CMake..."
cmake "${CMAKE_ARGS[@]}"

echo "[tui.sh] Building TUI target..."
cmake --build "$BUILD_DIR" -j "$JOBS" --target tui

# Optionally build publisher example (fastdds_sim_pub)
PUB_BIN=""
if [[ $RUN_PUB -eq 1 ]]; then
  echo "[tui.sh] Building DDS publisher (examples/fastdds_sim_pub)..."
  cmake --build "$BUILD_DIR" -j "$JOBS" --target fastdds_sim_pub || true
  PUB_CAN="${BUILD_DIR}/examples/fastdds_sim_pub/fastdds_sim_pub"
  if [[ -x "$PUB_CAN" ]]; then
    PUB_BIN="$PUB_CAN"
  fi
fi

BIN="${BUILD_DIR}/tui/tui"
if [[ ! -x "$BIN" ]]; then
  FOUND=$(find "$BUILD_DIR" -type f -executable -name tui 2>/dev/null | head -n1 || true)
  if [[ -n "$FOUND" ]]; then BIN="$FOUND"; fi
fi
if [[ ! -x "$BIN" ]]; then
  echo "[tui.sh] ERROR: TUI binary not found after build." >&2
  exit 1
fi

# Optionally run the DDS publisher in background
if [[ -n "$PUB_BIN" ]]; then
  echo "[tui.sh] Starting publisher: $PUB_BIN &"
  "$PUB_BIN" >/dev/null 2>&1 &
  PUB_PID=$!
  sleep 0.2
fi

echo "[tui.sh] Running: $BIN ${TUI_ARGS[*]:-}"
"$BIN" "${TUI_ARGS[@]:-}"

if [[ -n "${PUB_PID:-}" ]]; then
  kill "$PUB_PID" >/dev/null 2>&1 || true
fi
