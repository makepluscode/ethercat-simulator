#!/usr/bin/env bash
set -euo pipefail

# Sync KickCAT static lib and headers from Conan cache into libs/kickcat
# Usage:
#   bash tools/sync_kickcat.sh
# Env:
#   KICKCAT_REF      Conan reference (default: kickcat/v2.0-rc1)
#   CONAN_HOME       Conan home (default: <repo>/.conan2)

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
REPO_ROOT=$(cd -- "${SCRIPT_DIR}/.." &>/dev/null && pwd)

: "${KICKCAT_REF:=kickcat/v2.0-rc1}"
: "${CONAN_HOME:=${REPO_ROOT}/.conan2}"

if ! command -v conan >/dev/null 2>&1; then
  echo "[sync_kickcat] ERROR: conan not found. Install Conan v2 first." >&2
  exit 1
fi

echo "[sync_kickcat] Conan home: ${CONAN_HOME}"
echo "[sync_kickcat] KickCAT ref: ${KICKCAT_REF}"

if [ ! -f "${CONAN_HOME}/profiles/default" ]; then
  echo "[sync_kickcat] Detecting Conan profile..."
  CONAN_HOME="${CONAN_HOME}" conan profile detect
fi

echo "[sync_kickcat] Installing dependencies via Conan..."
CONAN_HOME="${CONAN_HOME}" \
KICKCAT_REF="${KICKCAT_REF}" \
conan install "${REPO_ROOT}" -of "${REPO_ROOT}/build" -s build_type=Release \
  -o ethercat-simulator*:with_kickcat=True --build=missing

echo "[sync_kickcat] Locating libkickcat.a in Conan cache..."
KLIB=$(find "${CONAN_HOME}" -type f -name libkickcat.a | head -n1 || true)
if [ -z "${KLIB}" ]; then
  echo "[sync_kickcat] ERROR: libkickcat.a not found in ${CONAN_HOME}." >&2
  exit 1
fi
KROOT=$(dirname "${KLIB}")/..
INCDIR="${KROOT}/include"

DEST_LIB="${REPO_ROOT}/libs/kickcat/lib"
DEST_INC="${REPO_ROOT}/libs/kickcat/include"
mkdir -p "${DEST_LIB}" "${DEST_INC}"

echo "[sync_kickcat] Copying library to ${DEST_LIB}"
cp -f "${KLIB}" "${DEST_LIB}/"

echo "[sync_kickcat] Syncing headers to ${DEST_INC}"
rsync -a --delete "${INCDIR}/" "${DEST_INC}/"

echo "[sync_kickcat] Done. CMake will use libs/kickcat if Conan package is not found."

