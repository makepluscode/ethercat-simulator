# Repository Guidelines

## Project Structure & Module Organization
- Core library code lives in `include/` and `core/`, covering Kickcat adapters and the simulator runtime.
- Applications ship from `src/a-master/` (EtherCAT master) and `src/a-slaves/` (virtual slaves). Examples and tests reside in `examples/` and `tests/`; optional UIs live under `tui/`, `gui/desktop/`, and `gui/web/`.
- Build tooling sits at the root (`CMakeLists.txt`, `conanfile.py`, helper scripts). Treat new modules as subdirectories with their own CMake targets.

## Build, Test, and Development Commands
- `./build.sh` builds `a-master` and `a-slaves` (use `--debug` for Debug, `--clean` to reset artifacts).
- `./test.sh` configures `build-tests/`, compiles all examples and GoogleTests, then runs `ctest`.
- Runtime basics: `build/src/a-slaves/a-slaves --uds /tmp/ethercat_bus.sock --count 1` and `build/src/a-master/a-master --uds /tmp/ethercat_bus.sock`. For remote sessions, switch to TCP options noted in the scripts.

## Coding Style & Naming Conventions
- C++17+, 4-space indent, LF endings, UTF-8. Prefer RAII and smart pointers; add `noexcept` when practical.
- Namespaces start at `ethercat_sim` with submodules like `simulation`, `communication`, or `kickcat`; avoid `using namespace` at file scope.
- Types use `PascalCase`, functions `camelCase`, files and namespaces `lower_snake_case`, constants/macros `UPPER_SNAKE_CASE`.
- Run `clang-format -i <files>` (LLVM style, 100 columns) before submitting.

## Testing Guidelines
- Tests live in `tests/<area>/test_*.cpp` and use GoogleTest. Name cases `SuiteName.MethodName_State_ExpectedBehavior`.
- Execute `ctest --test-dir build-tests --output-on-failure`. Target ≥80% coverage for simulation-critical logic; add regression tests when fixing bugs.

## Commit & Pull Request Guidelines
- Follow `type(scope): summary` commit messages (e.g., `feat(core): add virtual bus socket`) and reference issues like `#123` when relevant.
- Pull requests should summarize motivation, list key changes, include build/test evidence, and attach screenshots or logs for UI or simulation output.

## Security & Configuration Tips
- Simulator runs unprivileged on Linux/WSL; document any `sysctl` or capability tweaks if you enable real-time behavior.
- Avoid storing secrets; prefer environment variables or local config files for credentials or tokens.
