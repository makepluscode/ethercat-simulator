# EtherCAT Simulator

A lightweight EtherCAT simulator and scaffolding for experiments and integration. It provides a core simulation library, optional KickCAT master linkage via Conan, and small examples/tests to validate the toolchain.

## Project Layout
- Core: `include/`, `core/` (simulation, adapters)
- Communication (FastDDS): `include/communication/`, `core/communication/`
- Examples: `examples/hello/`, `examples/kickcat_scan/`, `examples/fastdds_smoke/`, `examples/fastdds_sim_pub/`, `examples/pdo_lrw/`, `examples/simul/`
- TUI: `tui/` (FTXUI-based UI with DDS subscribe)
- Docs: `PRD.md`, `AGENTS.md`
- Tests: `tests/` (GoogleTest via CTest)

## Prerequisites
- Linux/WSL
- CMake ≥ 3.20, C++17 toolchain (`build-essential` or similar)
- Conan v2 (recommended; used for GTest/KickCAT/FastDDS/FTXUI)

### Install Conan (recommended)
- pipx: `pipx install conan`
- or user install: `pip3 install --user conan`
- verify: `conan --version`

### Ubuntu toolchain
- `sudo apt-get update && sudo apt-get install -y build-essential cmake pkg-config`

## Build (CMake only)
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Build with Conan (GTest/KickCAT/FastDDS/FTXUI)
```
# 1) Detect profile and install deps
conan profile detect
export KICKCAT_REF=kickcat/v2.0-rc1  # optional: select KickCAT version
conan install . -of build -s build_type=Release \
  -o ethercat-simulator*:with_kickcat=True \
  -o ethercat-simulator*:with_fastdds=True \
  -o ethercat-simulator*:with_ftxui=True \
  --build=missing

# 2) Configure with Conan toolchain and CMakeDeps
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
  -DCMAKE_PREFIX_PATH=$PWD/build

# 3) Build & test
cmake --build build -j
ctest --test-dir build --output-on-failure
```
Notes
- FastDDS is required by default. Ensure `with_fastdds=True` is set on Conan install.
- `-DCMAKE_PREFIX_PATH=$PWD/build` lets CMake find `GTest`/`kickcat`/`fastdds` configs (Conan CMakeDeps).

### KickCAT Vendored Fallback (libs/kickcat)
- 미러링 스크립트로 Conan 캐시의 KickCAT 산출물을 `libs/kickcat`에 복사합니다.
```
bash tools/sync_kickcat.sh
```
- CMake는 `find_package(kickcat CONFIG)` 실패 시 `libs/kickcat`의 정적 라이브러리와 헤더를 자동 사용합니다.

## Run Examples
- Hello: `build/examples/hello/examples_hello`
- KickCAT scan: `build/examples/kickcat_scan/examples_kickcat_scan`
  - Uses a simulated socket layered over the internal `NetworkSimulator`. Current loopback stub reports 0 detected slaves; extend simulator for real discovery behavior.
 - FastDDS smoke: `build/examples/fastdds_smoke/examples_fastdds_smoke`
 - FastDDS pub: `build/examples/fastdds_sim_pub/fastdds_sim_pub "Hello DDS"`
 - PDO LRW: `build/examples/pdo_lrw/examples_pdo_lrw`

## TUI + DDS Quickstart
- Build & run TUI (DDS subscriber enabled by default):
  - `bash ./tui.sh`
- Also run DDS publisher in background:
  - `bash ./tui.sh --run-pub`
- Debug/clean build:
  - `bash ./tui.sh --debug --fresh`
The TUI subscribes to topic `sim_text` with `TextMsg` and shows the latest text published by `examples/fastdds_sim_pub` or `examples/simul`.

## Coding Style
- C++17+, 4-space indent, LF, UTF-8. Use `clang-format` (LLVM, 100 cols).
- Conventions and contribution rules: see `AGENTS.md`.

## Troubleshooting
- “Could not find fastdds”: run the Conan step with `-o ethercat-simulator*:with_fastdds=True` and pass `-DCMAKE_PREFIX_PATH=$PWD/build` to CMake.
- “Could not find GTest/kickcat”: ensure Conan step ran and pass `-DCMAKE_PREFIX_PATH=$PWD/build` on configure.
- Network/permissions: the simulator runs unprivileged; real NIC access (future) may require capabilities.

## Architecture Overview
```
Examples/Apps ──▶ KickCAT Master (Conan)
                 │
                 └──▶ SimSocket (Adapter)
                       │
                       └──▶ NetworkSimulator (Core)
                              ├──▶ Virtual Slaves (planned)
                              └──▶ Communication/FastDDS
```
- Adapter: `SimSocket` implements KickCAT `AbstractSocket`, bridging KickCAT frames to the simulator.
- Core: `NetworkSimulator` provides loopback today; extend to parse datagrams and emulate slaves.
- Extensibility: add fault injection (delay/drop/corrupt), timing, and DDS-based distribution.
