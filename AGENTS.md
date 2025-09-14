# Repository Guidelines

## Project Structure & Module Organization
- Root build files: `CMakeLists.txt`, `conanfile.py`.
- Core library (planned): `include/`, `core/`, `lib/`.
- Simulation: `core/simulation/` (network simulator, virtual slaves).
- Communication (FastDDS): `include/communication/`, `core/communication/`.
- GUI apps: `gui/desktop/` (Qt), `gui/web/` (HTML/JS/CSS).
- Examples/integration: `examples/`, `cmake_example/`; binaries in `build/examples/`.
- Docs/tests: `PRD.md`, `tests/` (CTest + GoogleTest).

## Build, Test, and Development Commands
- Use `build.sh` only; Conan toolchain is default.
- Build (Release): `./build.sh` (or `bash build.sh`).
- Debug: `./build.sh --debug`.
- Clean: `./build.sh --clean`.
- Run tests: `./test.sh`.
- Run examples (Linux/WSL): after build, execute binaries in `build/examples/`.

## Coding Style & Naming Conventions
- C++17+, 4-space indent, LF newlines, UTF‑8.
- Names: files/namespaces lower_snake_case; Types PascalCase; methods camelCase; constants/macros UPPER_SNAKE_CASE.
- Namespaces: use `ethercat_sim` as root; modules `simulation`, `communication`, `kickcat`. Prefer `ethercat_sim::kickcat::...`; legacy `ethercat_sim::kickcat_adapter` is deprecated.
- Avoid `using namespace` at file scope; use narrow `using` only in local scopes (tests/examples).
- Prefer RAII/smart pointers; avoid raw `new/delete`; add `noexcept` where sensible.
- Formatting: `clang-format` (LLVM, 100 cols). If `.clang-format` exists, run `clang-format -i <files>`.

## Testing Guidelines
- Framework: GoogleTest via CTest.
- Location: `tests/<area>/test_*.cpp`.
- Naming: `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage goal: ≥80% for core logic/simulation.
- Run: `ctest --test-dir build --output-on-failure`.

## Commit & Pull Request Guidelines
- Commits: concise, imperative; `type(scope): summary` (e.g., `feat(core): add cyclic operation thread`). Reference issues like `#123`.
- PRs: include summary, motivation, linked issues, build/test results, and relevant screenshots/logs (GUI/simulation). Keep diffs focused.

## Security & Configuration Tips
- Target Linux/WSL. Simulator needs no root; real NIC access may require capabilities.
- Real‑time tuning is optional; document any required `sysctl`/limits.
- Do not commit secrets; prefer env vars/local config.

## Agent‑Specific Instructions
- Follow this guide for all changes. If nested `AGENTS.md` files exist, deeper scopes take precedence.
- Validate locally via `build.sh` and `test.sh` before proposing PRs.
