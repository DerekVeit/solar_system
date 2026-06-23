# Solar System

C++20 simulation of solar system geometry and motion. Phase 0 provides a
fullscreen GLFW/OpenGL window, Keplerian ephemeris core, and unit tests.

## Requirements

- CMake 3.22+
- Ninja (recommended)
- g++-12 or clang++ 14
- GLFW 3, OpenGL 4.6, pkg-config

On Linux Mint / Ubuntu:

```bash
sudo apt install build-essential cmake ninja-build pkg-config \
    libglfw3-dev libgl1-mesa-dev
```

Other dependencies (fmt, glm, nlohmann/json, Catch2) are fetched automatically
by CMake.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=g++-12
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/src/solar_system
```

Press **Escape** to quit. The simulation clock advances one day per real-time
second while the window is open.