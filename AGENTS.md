# Repository Guidelines

## Project Structure & Module Organization
Core EtherCAT adapters and simulator runtime live in `include/` and `core/`. Runtime executables reside in `src/a-master/` for the master and `src/a-slaves/` for virtual devices, while reusable demos stay in `examples/`. Tests belong in `tests/`, and optional interfaces sit under `tui/`, `gui/desktop/`, or `gui/web/`. When introducing a module, place it in its own subdirectory, add a corresponding CMake target, and register it in the root `CMakeLists.txt` plus any helper scripts.

## Build, Test, and Development Commands
Run `./build.sh` to compile the master and slave binaries; pass `--debug` for Debug builds or `--clean` to reset artifacts. Execute `./test.sh` to configure `build-tests/`, build GoogleTests and examples, and invoke `ctest`. To exercise the stack manually, start `build/src/a-slaves/a-slaves --uds /tmp/ethercat_bus.sock --count 1` and then `build/src/a-master/a-master --uds /tmp/ethercat_bus.sock`.

## Coding Style & Naming Conventions
Write C++17+ with 4-space indentation, LF endings, and UTF-8 encoding. Prefer RAII abstractions and keep logic inside `ethercat_sim` namespaces. Use `PascalCase` for types, `camelCase` for functions, and `lower_snake_case` for files. Format changes with `clang-format -i` (LLVM style, 100 columns) before submitting.

## Testing Guidelines
GoogleTest powers unit and integration coverage. Place tests under `tests/<area>/test_*.cpp`, name them like `LinkState.Update_WhenBusDrops_RaisesFault`, and guard simulator-critical code with ≥80% coverage. Run `ctest --test-dir build-tests --output-on-failure` prior to any review or release.

## Commit & Pull Request Guidelines
Follow `type(scope): summary` commit messages, referencing issues such as `#123` when relevant. Pull requests should motivate the change set, enumerate key updates, and attach build or test evidence; include screenshots or logs for UI and runtime behavior. Note any required system tweaks (e.g., real-time `sysctl` settings) in the PR description.

## Security & Configuration Tips
Operate the simulator unprivileged on Linux or WSL. Keep secrets out of the repository and prefer environment variables or ignored local configuration files for credentials. Document any host configuration steps needed to enable deterministic or real-time performance.
