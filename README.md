# EtherCAT Simulator

A lightweight EtherCAT simulator with a core library and two primary applications:
- `a-master`: EtherCAT master backed by Kickcat, using a socket-based virtual bus
- `a-slaves`: multi-slave simulator (N virtual slaves in a single process)

## Project Layout
- Core: `include/`, `core/` (simulation, Kickcat adapters)
- Apps: `src/a-master/`, `src/a-slaves/`
- Examples: `examples/` (built via `test.sh`)
- TUI/GUI: `tui/`, `gui/`
- Docs: `PRD.md`, `AGENTS.md`
- Tests: `tests/` (GoogleTest via CTest)

## Prerequisites
- Linux/WSL
- CMake ≥ 3.20, C++17 toolchain
- Conan v2 (recommended; resolves Kickcat, GTest, FTXUI, optional FastDDS)

## Build (apps only)
```
./build.sh            # Release
./build.sh --debug    # Debug
./build.sh --clean    # Clean build dir
```
This builds only `a-master` and `a-slaves` into `build/src/...`.

## Tests & Examples
```
./test.sh                 # configure build-tests/, build, and run tests
./test.sh -L core         # by CTest label
./test.sh -R "EL1258|VirtualSlave"  # regex filter
```

## Run a-master and a-slaves
Local (UDS):
```
# Terminal 1 (slaves)
./a-slaves.sh --uds /tmp/ethercat_bus.sock --count 1

# Terminal 2 (master)
./a-master.sh --uds /tmp/ethercat_bus.sock
```
Remote (TCP):
```
# On slave machine
./a-slaves.sh --tcp 0.0.0.0:5510 --count 1

# On master machine
./a-master.sh --tcp <slave-host>:5510
```

Graceful exit: press ESC, Ctrl+C, or Ctrl+Z in either terminal.

## Notes
- Kickcat API is preserved; the virtual bus sends raw EtherCAT frames unmodified and accumulates WKC across the slave chain.
- Conan toolchain is used by default; avoid system packages.
- Optional FTXUI TUI and desktop GUI are available; see `tui/` and `gui/`.

## Coding Style
- C++17+, 4-space indent, LF, UTF-8. Use `clang-format` (LLVM, 100 cols).
- Contribution rules and conventions: see `AGENTS.md`.

## Troubleshooting
- Conan packages not found: re-run `conan install` (automated by scripts) and ensure `-DCMAKE_TOOLCHAIN_FILE` is set.
- Network/permissions: simulators run unprivileged; real NIC access (future) may require capabilities.
