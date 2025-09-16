# Repository Guidelines

## Project Structure & Module Organization
- Root build: `CMakeLists.txt`, `conanfile.py`.
- Core library: `include/`, `core/` (Kickcat adapters, simulator).
- Apps: `src/a-master/` (EtherCAT master), `src/a-slaves/` (N virtual slaves in one process).
- Examples/tests: `examples/`, `tests/` (built via `test.sh`).
- Optional UI: `tui/` (FTXUI), GUI: `gui/desktop/`, `gui/web/`.

## Build, Test, and Development Commands
- Build apps: `./build.sh` (Debug: `--debug`, Clean: `--clean`). Builds `a-master`, `a-slaves` only.
- Tests/Examples: `./test.sh` (creates `build-tests/`, builds examples + tests, runs CTest).
- Run slaves: `build/src/a-slaves/a-slaves --uds /tmp/ethercat_bus.sock --count 1`.
- Run master: `build/src/a-master/a-master --uds /tmp/ethercat_bus.sock`.
- Remote run: use TCP (`--tcp 0.0.0.0:5510` on slaves; `--tcp host:5510` on master).
 - Graceful exit: press ESC, Ctrl+C, or Ctrl+Z.

## Coding Style & Naming Conventions
- C++17+, 4-space indent, LF, UTF‑8; prefer RAII/smart pointers; add `noexcept` where sensible.
- Names: files/namespaces `lower_snake_case`; Types `PascalCase`; methods `camelCase`; constants/macros `UPPER_SNAKE_CASE`.
- Namespaces: root `ethercat_sim` with modules `simulation`, `communication`, `kickcat`; avoid file‑scope `using namespace`.
- Formatting: `clang-format` (LLVM, 100 cols). Run `clang-format -i <files>`.

## Testing Guidelines
- Framework: GoogleTest via CTest; run `ctest --test-dir build-tests --output-on-failure`.
- Location: `tests/<area>/test_*.cpp`; name tests `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage: target ≥80% for core logic/simulation.

## Commit & Pull Request Guidelines
- Commits: `type(scope): summary` (e.g., `feat(core): add virtual bus socket`); reference issues like `#123`.
- PRs: summary, motivation, linked issues, build/test results; add screenshots/logs for GUI/simulation when relevant.
- Keep diffs focused; update docs/tests when behavior changes.

## Security & Configuration Tips
- Target Linux/WSL. Simulator needs no root; real NIC access may require capabilities.
- Document any `sysctl`/limits if enabling real‑time.
- Do not commit secrets; use env vars/local config.
