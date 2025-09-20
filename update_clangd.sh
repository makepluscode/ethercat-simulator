#!/usr/bin/env bash
set -euo pipefail

# Quick script to update compile_commands.json for clangd
# This is useful when you make changes to CMakeLists.txt or want to refresh the index

BUILD_DIR="build"

echo "[clangd] Updating compile_commands.json..."

# Generate compile_commands.json
cmake -S . -B "${BUILD_DIR}" \
  -DBUILD_EXAMPLES=ON -DBUILD_TESTING=ON -DBUILD_MASTER=ON -DBUILD_SLAVES=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Copy to root directory
if [[ -f "${BUILD_DIR}/compile_commands.json" ]]; then
  cp "${BUILD_DIR}/compile_commands.json" .
  echo "[clangd] ✅ Updated compile_commands.json for IDE support"
  echo "[clangd] 📁 File size: $(du -h compile_commands.json | cut -f1)"
  echo "[clangd] 📄 Entries: $(grep -c '"file":' compile_commands.json)"
else
  echo "[clangd] ❌ Failed to generate compile_commands.json"
  exit 1
fi