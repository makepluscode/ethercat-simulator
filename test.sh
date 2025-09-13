#!/usr/bin/env bash
set -euo pipefail

# Simple test runner for GoogleTest via CTest
# Usage: test.sh [--debug|--release] [--conan|--no-conan] [--fresh] [--coverage] [-j N] [-- <ctest-args>]
# Examples:
#   bash ./test.sh --debug --fresh
#   bash ./test.sh --release -j 8 -- -R Communication
#   bash ./test.sh --debug --coverage

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
USE_CONAN=${USE_CONAN:-auto}  # auto|on|off
FRESH=0
# 기본: 커버리지 활성화 (요구사항: 기본 통합)
COVERAGE=${COVERAGE:-1}
USE_LLVM_COV=0

# Determine parallel jobs
if command -v nproc >/dev/null 2>&1; then
  JOBS_DEFAULT=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
  JOBS_DEFAULT=$(sysctl -n hw.ncpu)
else
  JOBS_DEFAULT=4
fi
JOBS=${JOBS:-$JOBS_DEFAULT}

CTEST_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --release) BUILD_TYPE=Release; shift ;;
    --debug)   BUILD_TYPE=Debug; shift ;;
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    --fresh)  FRESH=1; shift ;;
    --coverage) COVERAGE=1; shift ;;
    --no-coverage) COVERAGE=0; shift ;;
    --conan)   USE_CONAN=on; shift ;;
    --no-conan) USE_CONAN=off; shift ;;
    -j|--jobs) JOBS="$2"; shift 2 ;;
    --) shift; CTEST_ARGS+=("$@"); break ;;
    -h|--help)
      echo "Usage: $0 [--debug|--release] [--conan|--no-conan] [--fresh] [--coverage|--no-coverage] [-j N] [-- <ctest-args>]";
      exit 0 ;;
    *) CTEST_ARGS+=("$1"); shift ;;
  esac
done

CMAKE_TOOLCHAIN_ARG=()

if [[ $FRESH -eq 1 ]]; then
  rm -rf "$BUILD_DIR"
fi

if [[ "$USE_CONAN" != "off" ]] && command -v conan >/dev/null 2>&1 && [[ -f conanfile.py ]]; then
  # Best-effort Conan setup; ignore profile detect failure if already present
  conan profile detect || true
  conan install . -of "$BUILD_DIR" -s build_type="$BUILD_TYPE" --build=missing
  CMAKE_TOOLCHAIN_ARG=("-DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake")
fi

# Decide coverage tooling and flags
if [[ $COVERAGE -eq 1 ]]; then
  BUILD_TYPE=${BUILD_TYPE:-Debug}
  if ! command -v lcov >/dev/null 2>&1 && ! command -v gcovr >/dev/null 2>&1; then
    if command -v llvm-cov >/dev/null 2>&1 && command -v llvm-profdata >/dev/null 2>&1 && c++ --version 2>/dev/null | grep -qi clang; then
      USE_LLVM_COV=1
      echo "[test.sh] Using LLVM coverage tooling (llvm-cov/llvm-profdata)."
    else
      echo "[test.sh] Coverage tools not found (lcov/gcovr). Install them or install llvm-cov + use Clang."
    fi
  fi
fi

cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DBUILD_EXAMPLES=OFF -DBUILD_GUI=OFF \
  -DFORCE_NO_FTXUI=ON -DFORCE_NO_FASTDDS=ON \
  -DENABLE_COVERAGE=$([[ $COVERAGE -eq 1 ]] && echo ON || echo OFF) \
  -DENABLE_LLVM_COVERAGE=$([[ $USE_LLVM_COV -eq 1 ]] && echo ON || echo OFF) \
  ${CMAKE_TOOLCHAIN_ARG[@]:-}
cmake --build "$BUILD_DIR" -j "$JOBS"

# Zero counters before running tests to avoid stale data (gcov path)
if [[ $COVERAGE -eq 1 && $USE_LLVM_COV -eq 0 ]] && command -v lcov >/dev/null 2>&1; then
  lcov --quiet --directory "$BUILD_DIR" --zerocounters || true
fi

# For LLVM mode, set profile output path (one per process)
if [[ $COVERAGE -eq 1 && $USE_LLVM_COV -eq 1 ]]; then
  export LLVM_PROFILE_FILE="$BUILD_DIR/coverage-%p.profraw"
fi

ctest --test-dir "$BUILD_DIR" --output-on-failure "${CTEST_ARGS[@]}"

if [[ $COVERAGE -eq 1 ]]; then
  echo "[test.sh] Collecting coverage..."
  if [[ $USE_LLVM_COV -eq 1 ]]; then
    # Merge .profraw files
    if ls "$BUILD_DIR"/coverage-*.profraw >/dev/null 2>&1; then
      llvm-profdata merge -sparse -o "$BUILD_DIR/coverage.profdata" "$BUILD_DIR"/coverage-*.profraw
      # Find test binaries
      mapfile -t TEST_BINS < <(find "$BUILD_DIR/tests" -type f -executable -name "test_*" 2>/dev/null || true)
      if [[ ${#TEST_BINS[@]} -eq 0 ]]; then
        mapfile -t TEST_BINS < <(find "$BUILD_DIR" -type f -executable -name "test_*" 2>/dev/null || true)
      fi
      if [[ ${#TEST_BINS[@]} -gt 0 ]]; then
        echo "[test.sh] LLVM coverage summary:"
        llvm-cov report -instr-profile="$BUILD_DIR/coverage.profdata" \
          -ignore-filename-regex '/usr/|\.conan2/|/conan/|/tests/|/examples/' \
          "${TEST_BINS[@]}" || true
        if command -v llvm-cov >/dev/null 2>&1; then
          rm -rf "$BUILD_DIR/coverage_html" || true
          llvm-cov show -format=html -output-dir "$BUILD_DIR/coverage_html" \
            -instr-profile="$BUILD_DIR/coverage.profdata" \
            -ignore-filename-regex '/usr/|\.conan2/|/conan/|/tests/|/examples/' \
            "${TEST_BINS[@]}" >/dev/null 2>&1 || true
          echo "[test.sh] HTML report: $BUILD_DIR/coverage_html/index.html"
        fi
      else
        echo "[test.sh] No test binaries found for llvm-cov report."
      fi
    else
      echo "[test.sh] No .profraw files found. Ensure Clang is used and tests ran."
    fi
  elif command -v lcov >/dev/null 2>&1; then
    # Capture
    GCOV_TOOL_ARGS=()
    if command -v llvm-cov >/dev/null 2>&1 && c++ --version 2>/dev/null | grep -qi clang; then
      GCOV_TOOL_ARGS=(--gcov-tool "llvm-cov gcov")
    fi
    # Use rc to avoid geninfo warnings about unexecuted blocks; ignore common parse warnings
    # Also filter out geninfo WARNING lines from stderr for a clean log
    {
      lcov --quiet --directory "$BUILD_DIR" --capture --output-file "$BUILD_DIR/coverage.info" \
        --ignore-errors mismatch,empty,gcov,source \
        --rc geninfo_unexecuted_blocks=1 \
        "${GCOV_TOOL_ARGS[@]}" \
        || true
    } 2> >(grep -v -E 'geninfo:|\(use\s+"geninfo')
    # Filter out externals and tests
    {
      lcov --quiet --remove "$BUILD_DIR/coverage.info" \
        '/usr/*' '*/.conan2/*' '*/conan/*' '*/tests/*' '*/examples/*' \
        --ignore-errors unused \
        -o "$BUILD_DIR/coverage.filtered.info" \
        || true
    } 2> >(grep -v '^lcov: WARNING') | grep -v -E '\(use\s+"lcov --ignore-errors'
    echo "[test.sh] Coverage summary (filtered):"
    lcov --summary "$BUILD_DIR/coverage.filtered.info" || true
    if command -v genhtml >/dev/null 2>&1; then
      genhtml --quiet -o "$BUILD_DIR/coverage_html" "$BUILD_DIR/coverage.filtered.info" || true
      echo "[test.sh] HTML report: $BUILD_DIR/coverage_html/index.html"
    fi
  elif command -v gcovr >/dev/null 2>&1; then
    # Fallback to gcovr text + HTML if available
    echo "[test.sh] Using gcovr fallback"
    gcovr -r . --exclude 'tests' --exclude 'examples' --print-summary || true
    gcovr -r . --exclude 'tests' --exclude 'examples' --html-details -o "$BUILD_DIR/coverage_html/index.html" || true
    echo "[test.sh] HTML report: $BUILD_DIR/coverage_html/index.html"
  else
    echo "[test.sh] Coverage tools not found (lcov/gcovr). Install to generate reports."
  fi
fi
