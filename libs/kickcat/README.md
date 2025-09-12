# KickCAT (vendored)

This folder mirrors the headers and static library of KickCAT installed via Conan.

How it is populated
- Run the Conan install step (see repository README) to build/fetch KickCAT.
- A helper step copies artifacts to this folder:
  - Library: `lib/libkickcat.a`
  - Headers: `include/kickcat/`

Build integration
- CMake first tries `find_package(kickcat CONFIG)`. If not found, it falls back to this directory and creates `kickcat::kickcat` imported target.

Refresh
- Re-run Conan install, then copy again to update this mirror.

