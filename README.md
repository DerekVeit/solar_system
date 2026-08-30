# Solar System

C++20 interactive simulation of solar system geometry and motion. Bodies move
on Keplerian orbits in an ecliptic J2000 frame and are drawn in a GLFW/OpenGL
window: textured spheres and rings, orbit trails, a galactic star map with
constellation figures, and an orbit camera with pan, zoom, and body follow.

![Earth view](docs/Earth%20at%202026-08-30%2014-12-29.png)
*Earth at 2026-08-30 14:12:29 PDT*

[More images](docs)

## Features

- **Keplerian ephemeris** — osculating elements for the Sun, eight planets,
  Pluto, and major moons (J2000-era data in `assets/data/bodies.json`)
- **Simulation clock** — paused, real-time, or accelerated time scales
- **3D ecliptic view** — lit spheres (with optional rings), grey orbit loops,
  fading motion tails, and a directional star background
- **Sky catalog** — HIP-keyed stars, named reticles/labels, and constellation
  plus asterism stick figures
- **Camera** — yaw/pitch orbit, pan, zoom, Home reset, and follow a body
  (`0`–`9`, `M` / `,` / `.`)
- **Presentation config** — surface shading and textures, rings, tail length,
  size scaling, and visibility in `assets/data/body_visuals.json`

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

The app starts in real time at the current wall-clock epoch. **T** switches to
accelerated time; **-** / **+** then halve or double the acceleration.

## Controls

| Key | Action |
|-----|--------|
| **Esc** | Quit |
| **Space** | Pause time |
| **R** | Real-time scale |
| **T** | Accelerated time scale |
| **-** / **+** (or numpad) | Halve / double acceleration |
| **Page Up** / **Page Down** | Zoom in / out |
| **Insert** | Frame the followed body so it fills the view |
| **Arrow keys** | Pan camera (or offset while following) |
| **W** / **S** | Pitch up / down |
| **A** / **D** | Yaw left / right |
| **Numpad 9 / 1 / 6 / 4** | View target from celestial N / S / E / W |
| **Numpad 8 / 2** | View target from celestial +Y / -Y |
| **Home** | Clear follow, reset camera, center view on the Sun |
| **End** | Reset camera orientation and distance (keep look-at) |
| **`** | Clear follow, but keep current view |
| **0**–**9** | Follow Sun through Pluto |
| **M** | Follow the innermost satellite of the current body |
| **,** / **.** | Previous / next sibling (same primary; planets when following a planet) |
| **B** / **Shift+B** | Body size scaling off / on |
| **C** / **X** | Named-star reticles and labels on / off |
| **\\** | Toggle body graticules |

Constellation and asterism stick figures are on when `constellations.json`
has `"visible": true`; there is no key to toggle them yet.

## Data

| File | Role |
|------|------|
| `assets/data/bodies.json` | Physical/orbital definitions (mu, radius, Kepler elements, pole, rotation, optional `primary`) |
| `assets/data/body_visuals.json` | Defaults + per-body surface, textures, rings, tails, size, visibility, moon orbit scale |
| `assets/data/sky.json` | Star-map texture, galactic longitude offset, brightness |
| `assets/data/stars.json` | HIP-keyed J2000 RA/Dec stars; optional names draw reticles and labels |
| `assets/data/constellations.json` | 40 IAU showpiece constellations plus 7 asterisms as HIP polylines |
| `assets/textures/` | Globe, ring, and sky maps (see `assets/textures/ATTRIBUTION.md`) |

## Layout

| Path | Role |
|------|------|
| `src/core/` | Ephemeris types, Kepler math, body orientation, JSON body loader |
| `src/sim/` | Simulation clock and facade over the ephemeris |
| `src/app/` | Window, input, scene, OpenGL renderer |
| `assets/` | Runtime data and textures (copied beside the binary) |
| `tests/` | Catch2 unit tests |
| `docs/` | Developer guide and screenshots |

## How this was written

Personal project to rebuild C++ and learn OpenGL, with a later path toward
Vulkan. It grew from a long working conversation with Grok Build, not a
one-shot prompt. I set the goals and design, and wrote or reworked much of
the early core — types, Kepler, clock, windowing, first drawing — while
using the sessions as a study guide. As the simulation took shape I
increasingly asked Grok Build to implement whole features after we had
agreed the approach, then reviewed and steered those changes.
