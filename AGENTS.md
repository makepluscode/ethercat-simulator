# Repository Guidelines

## Project Structure & Module Organization
- Root build: `CMakeLists.txt`, `conanfile.py`.
- Core library (planned): `include/`, `core/`, `lib/`.
- Simulation: `core/simulation/` (network simulator, virtual slaves).
- Communication (FastDDS): `include/communication/`, `core/communication/`.
- GUI apps: `gui/desktop/` (Qt), `gui/web/` (HTML/JS/CSS).
- Examples/integration: `examples/`, `cmake_example/`.
- Docs: `PRD.md`. Tests: `tests/` (via CTest when added).

## Build, Test, and Development Commands
- Configure (Release): `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`.
- Build: `cmake --build build -j`.
- Run tests: `ctest --test-dir build --output-on-failure`.
- With Conan deps:
  - `conan profile detect`
  - `conan install . -of build -s build_type=Release --build=missing`
  - `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake`
  - `cmake --build build -j && ctest --test-dir build`
- Examples output: `build/examples/` (run binaries directly on Linux/WSL).

## Coding Style & Naming Conventions
- Language: C++17+. Indentation: 4 spaces; LF newlines; UTF‑8.
- Files: lower_snake_case (e.g., `network_interface.h`, `master.cpp`).
- Namespaces: lower_snake_case (e.g., `kickcat::communication`).
- Types: PascalCase; methods: camelCase; constants/macros: UPPER_SNAKE_CASE.
- Prefer RAII and smart pointers; avoid raw `new/delete`; use `noexcept` when sensible.
- Formatting: `clang-format` (LLVM style, 100 columns). If `.clang-format` exists, follow it.

## Testing Guidelines
- Framework: GoogleTest via CTest. Place tests under `tests/<area>/test_*.cpp`.
- Naming: `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage goal: ≥80% for core logic and simulation.
- Run tests: `ctest --test-dir build --output-on-failure`.

## Commit & Pull Request Guidelines
- Commits: concise, imperative. Format: `type(scope): summary` (e.g., `feat(core): add cyclic operation thread`). Reference issues with `#123`.
- Group related changes; avoid mixed concerns.
- PRs: include summary, motivation, linked issues, build/test results, and screenshots/logs where relevant (GUI/simulation). Keep diffs focused.

## Security & Configuration Tips
- Target: Linux/WSL. Simulator needs no root; real NIC access may require capabilities.
- Real‑time tuning optional; document any required `sysctl`/limits for timing‑sensitive work.
- Do not commit secrets; prefer env vars/local config.

## Agent‑Specific Notes
- Follow this guide for any file you modify. Keep changes minimal and aligned with `PRD.md` and the layout above.
- Respect directory scope if additional `AGENTS.md` files appear; deeper files take precedence.
- Validate locally with CMake/CTest before opening PRs.

