# Repository Guidelines

## Project Structure & Module Organization
- Root build files: `CMakeLists.txt`, `conanfile.py`.
- Core library (planned): `include/`, `core/`, `lib/`.
- Simulation: `core/simulation/` (network simulator, virtual slaves).
- Communication (FastDDS): `include/communication/`, `core/communication/`.
- GUI apps: `gui/desktop/` (Qt), `gui/web/` (HTML/JS/CSS).
- Examples/integration: `examples/`, `cmake_example/`. Binaries in `build/examples/`.
- Docs/tests: `PRD.md`, `tests/` (CTest + GoogleTest).

## Build, Test, and Development Commands
- Configure (Release): `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- Build: `cmake --build build -j`
- Run tests: `ctest --test-dir build --output-on-failure`
- With Conan deps:
  - `conan profile detect`
  - `conan install . -of build -s build_type=Release --build=missing`
  - `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake`
  - `cmake --build build -j && ctest --test-dir build`
- Run examples (Linux/WSL): execute binaries in `build/examples/`.

## Coding Style & Naming Conventions
- C++17+, 4-space indent, LF newlines, UTF‑8.
- Files/namespaces: lower_snake_case. Types: PascalCase. Methods: camelCase. Constants/macros: UPPER_SNAKE_CASE.
- Namespaces: root `ethercat_sim`; modules `simulation`, `communication`, `kickcat` (adapter). Legacy alias `ethercat_sim::kickcat_adapter` is provided but deprecated. Always qualify from root (e.g., `ethercat_sim::kickcat::SimSocket`) to avoid collision with external `::kickcat`.
- Avoid `using namespace` at file scope. Prefer explicit qualification or narrow `using` declarations for single symbols in local scopes (tests/examples only).
- Prefer RAII and smart pointers; avoid raw `new/delete`; use `noexcept` where sensible.
- Formatting: `clang-format` (LLVM, 100 cols). If `.clang-format` exists, run `clang-format -i <files>`.

## Testing Guidelines
- Framework: GoogleTest via CTest.
- Location: `tests/<area>/test_*.cpp`.
- Naming: `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage goal: ≥80% for core logic/simulation.
- Run: `ctest --test-dir build --output-on-failure`.

## Commit & Pull Request Guidelines
- Commits: concise, imperative; `type(scope): summary` (e.g., `feat(core): add cyclic operation thread`). Reference issues `#123`.
- PRs: include summary, motivation, linked issues, build/test results, and screenshots/logs for GUI/simulation. Keep diffs focused.

## Security & Configuration Tips
- Target Linux/WSL. Simulator needs no root; real NIC access may require capabilities.
- Real‑time tuning is optional; document any required `sysctl`/limits.
- Do not commit secrets; prefer env vars/local config.

## Agent‑Specific Notes
- Follow this guide for any changes. Respect directory scope if nested AGENTS.md files appear. Validate locally with CMake/CTest before proposing PRs.
