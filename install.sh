#!/usr/bin/env bash
set -euo pipefail

# Install all third-party libraries for this project via Conan, except KickCAT.
# KickCAT headers/libs must live inside the repository (libs/kickcat) for include references.
# Optionally, pass --vendor-kickcat to mirror KickCAT from Conan cache into libs/kickcat.
#
# Usage:
#   bash ./install.sh [--debug|--release] [--fresh] [--vendor-kickcat] \
#                     [--kickcat-ref <ref>] [--fastdds-ref <ref>] [--build-dir <dir>] [--] [extra conan args]
#
# Examples:
#   bash ./install.sh --release
#   bash ./install.sh --debug --vendor-kickcat --kickcat-ref kickcat/v2.0-rc1
#
# After running:
#   cmake -S . -B build \
#     -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
#     -DCMAKE_PREFIX_PATH=$PWD/build

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Release}
FRESH=0
VENDOR_KICKCAT=0
KICKCAT_REF=${KICKCAT_REF:-}
FASTDDS_REF=${FASTDDS_REF:-}

CONAN_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --release) BUILD_TYPE=Release; shift ;;
    --debug)   BUILD_TYPE=Debug; shift ;;
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    --build-dir) BUILD_DIR="$2"; shift 2 ;;
    --fresh)   FRESH=1; shift ;;
    --vendor-kickcat) VENDOR_KICKCAT=1; shift ;;
    --kickcat-ref) KICKCAT_REF="$2"; shift 2 ;;
    --fastdds-ref) FASTDDS_REF="$2"; shift 2 ;;
    --) shift; CONAN_ARGS+=("$@"); break ;;
    -h|--help)
      sed -n '1,60p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) CONAN_ARGS+=("$1"); shift ;;
  esac
done

if ! command -v conan >/dev/null 2>&1; then
  echo "[install.sh] ERROR: Conan v2 is required. Install via 'pipx install conan' or 'pip3 install --user conan'." >&2
  exit 1
fi

if [[ $FRESH -eq 1 ]]; then
  rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

echo "[install.sh] Conan profile detection (best effort)"
conan profile detect || true

# Install required libs: GTest (via conanfile), FastDDS, FTXUI. Skip KickCAT on purpose.
echo "[install.sh] Installing libraries (FastDDS + FTXUI, no KickCAT) to $BUILD_DIR (type=$BUILD_TYPE)"
if [[ -n "$FASTDDS_REF" ]]; then
  export FASTDDS_REF
  echo "[install.sh] Using FASTDDS_REF=$FASTDDS_REF"
fi
conan install . -of "$BUILD_DIR" -s build_type="$BUILD_TYPE" \
  -o ethercat-simulator*:with_fastdds=True \
  -o ethercat-simulator*:with_ftxui=True \
  -o ethercat-simulator*:with_kickcat=False \
  --build=missing ${CONAN_ARGS[*]:-}

# Optionally mirror KickCAT from Conan cache into libs/kickcat
if [[ $VENDOR_KICKCAT -eq 1 ]]; then
  if [[ -z "$KICKCAT_REF" ]]; then
    KICKCAT_REF=kickcat/v2.0-rc1
  fi
  echo "[install.sh] Vendoring KickCAT ($KICKCAT_REF) into libs/kickcat via tools/sync_kickcat.sh"
  KICKCAT_REF="$KICKCAT_REF" bash ./tools/sync_kickcat.sh
else
  if [[ ! -d "libs/kickcat/include" ]]; then
    echo "[install.sh] NOTICE: libs/kickcat/include not found."
    echo "             KickCAT must be present in the repository for headers."
    echo "             You can vendor it now with: bash ./install.sh --vendor-kickcat [--kickcat-ref <ref>]"
  fi
fi

echo "[install.sh] Done. Toolchain: $BUILD_DIR/conan_toolchain.cmake"
echo "[install.sh] Next:"
echo "  cmake -S . -B $BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake -DCMAKE_PREFIX_PATH=$PWD/$BUILD_DIR"
