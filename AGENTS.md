# Repository Guidelines

## Project Structure & Module Organization
- Root CMake project with Conan packaging: `CMakeLists.txt`, `conanfile.py`.
- Core library (planned): `include/`, `core/`, `lib/`.
- Simulation: `core/simulation/` (network simulator, virtual slaves).
- Communication (FastDDS): `include/communication/`, `core/communication/`.
- GUI apps: `gui/desktop/` (Qt), `gui/web/` (HTML/JS/CSS).
- Examples and integration: `examples/`, `cmake_example/`.
- Docs: `PRD.md` (requirements). Tests live under `tests/` (when added) and run via CTest.

## Build, Test, and Development Commands
- Configure (Release): `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- Build: `cmake --build build -j`
- Run tests: `ctest --test-dir build --output-on-failure`
- With Conan deps:
  - `conan profile detect`
  - `conan install . -of build -s build_type=Release --build=missing`
  - `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake`
  - `cmake --build build -j && ctest --test-dir build`
- Examples: built under `build/examples/`; run binaries directly (Linux/WSL).

## Coding Style & Naming Conventions
- Language: C++17+. Indentation: 4 spaces, LF newlines, UTF-8.
- Files: lower_snake_case (`network_interface.h`, `master.cpp`).
- Namespaces: lower_snake_case (`kickcat::communication`).
- Types: PascalCase (`Master`, `VirtualSlave`, `EtherCATFrame`).
- Functions/methods: camelCase (`sendFrame`, `initialize`).
- Constants/macros: UPPER_SNAKE_CASE.
- Prefer RAII, smart pointers, `noexcept` where appropriate; avoid raw new/delete.
- Formatting: use `clang-format` (LLVM style, 100 col). If a `.clang-format` is added, follow it.

## Testing Guidelines
- Framework: GoogleTest via CTest integration (planned). Place unit tests in `tests/<area>/test_*.cpp`.
- Test names: `SuiteName.MethodName_State_ExpectedBehavior`.
- Coverage: target >=80% for core logic and simulation; add tests with new features.
- Run: `ctest --test-dir build` after building; add tests with `add_test` in CMake.

## Commit & Pull Request Guidelines
- Commits: concise, imperative. Recommended format: `type(scope): summary` (e.g., `feat(core): add cyclic operation thread`).
- Reference issues with `#123`. Group related changes; avoid mixed concerns.
- PRs: include summary, motivation, linked issues, build/test results, and, if applicable, screenshots (GUI) or logs (simulation runs). Keep diffs focused.

## Security & Configuration Tips
- Linux/WSL target. Simulator does not require root; real NIC access may need capabilities.
- Real-time tuning is optional for dev; document any required sysctl/limits in PRs touching timing.

## Agent-Specific Notes
- Follow this guide for any file you modify. Keep changes minimal, scoped, and aligned with the PRD and directory structure above.

