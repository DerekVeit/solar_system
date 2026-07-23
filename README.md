# Solar System

C++20 interactive simulation of solar system geometry and motion. Bodies move
on Keplerian orbits in the ecliptic plane and are drawn in a GLFW/OpenGL window
with orbit trails, camera pan/zoom, and optional body follow.

## Features

- **Keplerian ephemeris** — osculating elements for the Sun, eight planets, and
  Pluto (J2000-era data in `assets/data/bodies.json`)
- **Simulation clock** — paused, real-time, or accelerated time scales
- **Ecliptic view** — point sprites for bodies, grey orbit loops, fading motion
  tails
- **Camera** — pan, zoom, Home reset, and follow a body (`0`–`9`)
- **Presentation config** — surface shading (color, ambient, emission), tail
  length, size scaling, and visibility in `assets/data/body_visuals.json`

For a map of the codebase aimed at new contributors, see
[docs/DEVELOPER.md](docs/DEVELOPER.md).

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

The build copies `assets/` next to the `solar_system` executable.

## Run

```bash
./build/src/solar_system
```

The app starts in accelerated time (default 16 simulation days per real second)
at the current wall-clock epoch.

## Controls

| Key | Action |
|-----|--------|
| **Esc** | Quit |
| **Space** | Pause time |
| **R** | Real-time scale |
| **A** | Accelerated time scale |
| **-** / **+** (or numpad) | Halve / double acceleration |
| **Page Up** / **Page Down** | Zoom in / out |
| **Arrow keys** | Pan camera (or offset while following) |
| **W** / **S** | Pitch up / down |
| **A** / **D** | Yaw left / right |
| **Numpad 8 / 2 / 6 / 4** | View target from celestial N / S / E / W |
| **Home** | Clear follow, center view on the Sun |
| **End** | Reset camera orientation (keep target) |
| **`** | Clear follow, but keep current view |
| **0**–**9** | Follow Sun through Pluto (catalog order) |
| **B** / **Shift+B** | Body size scaling off / on |

## Data

| File | Role |
|------|------|
| `assets/data/bodies.json` | Physical/orbital definitions (mu, radius, Kepler elements) |
| `assets/data/body_visuals.json` | Defaults + per-body surface, tails, size, visibility |

## Layout

| Path | Role |
|------|------|
| `src/core/` | Ephemeris types, Kepler math, JSON body loader |
| `src/sim/` | Simulation clock and facade over the ephemeris |
| `src/app/` | Window, input, scene, OpenGL renderer |
| `assets/` | Runtime data (copied beside the binary) |
| `tests/` | Catch2 unit tests |
| `docs/` | Developer-oriented documentation |
