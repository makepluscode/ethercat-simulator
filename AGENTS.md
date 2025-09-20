# Repository Guidelines

## Project Structure & Module Organization
- Core logic resides in `include/` and `core/`, covering Kickcat adapters and the simulator runtime.
- Runtime applications live in `src/a-master/` (EtherCAT master) and `src/a-slaves/` (virtual slaves); examples sit in `examples/`, tests in `tests/`, and optional UIs under `tui/`, `gui/desktop/`, and `gui/web/`.
- Treat new modules as subdirectories with dedicated CMake targets; update the root `CMakeLists.txt` and related build scripts accordingly.

## Build, Test, and Development Commands
- `./build.sh` builds `a-master` and `a-slaves`; pass `--debug` for Debug artifacts and `--clean` to reset outputs.
- `./test.sh` configures `build-tests/`, compiles examples and GoogleTests, then invokes `ctest`.
- Run the stack locally with `build/src/a-slaves/a-slaves --uds /tmp/ethercat_bus.sock --count 1` followed by `build/src/a-master/a-master --uds /tmp/ethercat_bus.sock`.

## Coding Style & Naming Conventions
- Use C++17+ with 4-space indentation, LF endings, UTF-8 encoding, and RAII-centric design.
- Namespaces start at `ethercat_sim`; types use `PascalCase`, functions `camelCase`, and files `lower_snake_case`.
- Format C++ with `clang-format -i <files>` (LLVM style, 100 columns) before submitting.

## Testing Guidelines
- Tests reside in `tests/<area>/test_*.cpp` and use GoogleTest.
- Name tests `SuiteName.MethodName_State_ExpectedBehavior`; aim for ≥80% coverage on simulation-critical logic.
- Execute `ctest --test-dir build-tests --output-on-failure` to validate before pushing.

## Commit & Pull Request Guidelines
- Follow `type(scope): summary` commit messages (e.g., `feat(core): add virtual bus socket`) and reference issues like `#123` when applicable.
- Pull requests should describe motivation, list key changes, include build/test evidence, and attach logs or screenshots for UI or simulation output.

## Security & Configuration Tips
- Run the simulator unprivileged on Linux/WSL; document any `sysctl` or capability adjustments when enabling real-time behavior.
- Avoid storing secrets in the repo; prefer environment variables or local config files for credentials.
