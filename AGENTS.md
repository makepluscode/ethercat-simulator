# Repository Guidelines

## Project Structure & Module Organization
- Root build files: `CMakeLists.txt`, `conanfile.py`.
- Core library: `include/`, `core/`, `lib/`.
- Simulation: `core/simulation/` (network simulator, virtual slaves).
- Communication (Fast DDS): `include/communication/`, `core/communication/`.
- GUI apps: `gui/desktop/` (Qt), `gui/web/` (HTML/JS/CSS).
- Examples/integration: `examples/`, `cmake_example/`; binaries under `build/examples/`.
- Docs/tests: `PRD.md`, `tests/` (CTest + GoogleTest).

## Build, Test, and Development Commands
- Build Release: `./build.sh`; Debug: `./build.sh --debug`; Clean: `./build.sh --clean`.
- Tests: `./test.sh` (includes TUI smoke test via `TUI_SMOKE_TEST=1`).
- Run examples (Linux/WSL): execute binaries in `build/examples/`.
- Reproducibility: Conan toolchain is default; avoid system packages.

## Coding Style & Naming Conventions
- C++17+, 4-space indent, LF newlines, UTF‑8; prefer RAII/smart pointers; add `noexcept` where sensible.
- Names: files/namespaces lower_snake_case; Types PascalCase; methods camelCase; constants/macros UPPER_SNAKE_CASE.
- Namespaces: root `ethercat_sim` with modules `simulation`, `communication`, `kickcat`; prefer `ethercat_sim::kickcat::...`; legacy `ethercat_sim::kickcat_adapter` is deprecated. Avoid `using namespace` at file scope.
- Formatting: `clang-format` (LLVM, 100 cols). If `.clang-format` exists, run `clang-format -i <files>`.

## Testing Guidelines
- Framework: GoogleTest via CTest.
- Location: `tests/<area>/test_*.cpp`.
- Naming: `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage: target ≥80% for core logic/simulation.
- Run: `ctest --test-dir build --output-on-failure`.

## Commit & Pull Request Guidelines
- Commits: concise, imperative `type(scope): summary` (e.g., `feat(core): add cyclic operation thread`); reference issues like `#123`.
- PRs: include summary, motivation, linked issues, build/test results, and relevant screenshots/logs (GUI/simulation). Keep diffs focused.

## Security & Configuration Tips
- Target Linux/WSL. Simulator needs no root; real NIC access may require capabilities.
- Real‑time tuning is optional; document any required `sysctl`/limits. Do not commit secrets; use env vars/local config.

## Dependency & DDS Notes
- Fast DDS 3.x (`fast-dds/3.2.1`) and other deps pinned via Conan; coordinate any bump (update code, refresh `conan.lock`, run tests).
- Conan is required; `build.sh` writes CMake user presets into `build/` to avoid repo churn.

## Agent‑Specific Instructions
- This guide applies repo‑wide; deeper `AGENTS.md` files override within their subtree.
- Validate locally via `./build.sh` and `./test.sh` before proposing PRs or larger refactors.

