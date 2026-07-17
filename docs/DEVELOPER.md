# Developer guide

Orientation for someone new to this repository: how the pieces fit together,
where to look when changing behavior, and a few conventions that matter.

## High-level architecture

```
main → make_app / run_loop
         │
         ├─ Window (GLFW) + input callbacks
         ├─ SolarSystem (clock + EphemerisProvider)
         └─ Scene (BodyVisuals + camera + IRenderer)
                    │
                    └─ each frame: ViewFrame → BodyVisual::append_draw → DrawBatch → GlRenderer
```

Three layers keep physics, presentation, and windowing separate:

| Layer | Namespace | Responsibility |
|-------|-----------|----------------|
| **Core** | `solar::core` | Bodies, epochs, Kepler propagation, ephemeris interface |
| **Simulation** | `solar::sim` | Clock / time scale and a thin facade over the ephemeris |
| **App** | `solar::app` | Window, input, scene graph visuals, OpenGL drawing |

The simulation does not know about OpenGL. Rendering never mutates orbital
elements; it only samples positions at the current (or historical) epoch.

## Directory map

```
src/
  core/           # pure geometry / ephemeris (no GLFW)
  sim/            # clock + SolarSystem facade
  app/
    main.cpp      # entry: make_app, run_loop, shutdown log
    run.cpp       # app construction and main loop
    window.*      # GLFW window, clear color, framebuffer size
    input.*       # key bindings
    follow_targets.hpp  # digit → body name for camera follow
    context.hpp   # pointers shared with GLFW callbacks
    scene/        # camera + body visuals + draw batching
    render/       # IRenderer, GlRenderer, GPU types
assets/data/      # bodies.json, body_visuals.json
tests/            # Catch2 tests against solar_core (+ some app types)
```

CMake builds:

- **`solar_core`** — static library (`core/` + `sim/`)
- **`solar_system`** — executable (`app/` + glad + GLFW)
- **tests** — link `solar_core` (and a few app sources as needed)

Assets are copied next to the executable on build (`POST_BUILD` in
`src/CMakeLists.txt`). Runtime paths go through `app/files.hpp`
(`asset_path(...)`).

---

## Core (`src/core/`)

### Types (`types.hpp`)

Fundamental values: `Epoch` (Julian date), `Displacement` / `Velocity` /
`StateVector` (glm `dvec3` in km and km/s), `KeplerianElements`, polar helpers.

### Constants (`constants.hpp`)

AU, day length, angle conversions, J2000 JD, etc.

### Kepler (`kepler.hpp` / `kepler.cpp`)

Mean motion, period, Kepler equation solver, state from elements at an epoch,
helpers used for orbit sampling and motion tails
(`epoch_before_mean_anomaly`, `mean_anomaly_at_epoch`, …).

### Ephemeris interface (`ephemeris.hpp` / `ephemeris.cpp`)

- `BodyDefinition` — name, mu, radius, elements
- `EphemerisProvider` — `state(name, epoch)` and `bodies()`
- `find_body`, `central_gravitational_parameter` (Sun’s mu)

### Kepler ephemeris (`kepler_ephemeris.*`)

Concrete provider: for the Sun (or any body with `a_km <= 0`) returns a zero
state; otherwise `state_from_kepler` with the central mu.

### JSON loader (`json_loader.*`)

Loads `assets/data/bodies.json` into `std::vector<BodyDefinition>`.

**When changing orbits or adding bodies:** update `bodies.json` and tests that
assert catalog contents; keep Sun first with a valid mu (required by the
Kepler ephemeris).

---

## Simulation (`src/sim/`)

### Clock (`clock.*`)

`SimulationClock` holds the current `Epoch` and a `TimeScale`:

- `paused` — no advance
- `real_time` — 1 sim second per wall second
- `accelerated` — wall delta × `acceleration_` (default set in `make_app` to
  16 days/s)

`advance(delta_seconds)` is called once per frame from the main loop.

### Solar system (`solar_system.*`)

Owns the ephemeris and clock. `state(body_name)` is shorthand for
`ephemeris.state(name, clock.epoch())`.

**When changing time behavior:** clock only. **When changing how positions are
computed:** ephemeris / Kepler, not the clock.

---

## App: lifecycle (`src/app/`)

### Entry (`main.cpp`, `run.*`)

1. `make_app()` — load bodies, build ephemeris + clock, populate scene from
   visuals JSON, create window, register keys, `scene.init()`
2. `run_loop()` — advance clock, clear, `scene.render(...)`, swap, poll
3. `log_shutdown_report()` — sample positions / epoch on exit

### Window (`window.*`)

GLFW setup, user pointer, key callback hook, framebuffer size, clear color,
alpha blending enable for trails.

### Input (`input.*`, `follow_targets.hpp`)

GLFW key callback via `AppContext` (window / simulation / scene pointers).
Follow targets `0`–`9` are a fixed name table aligned with `bodies.json` order;
validation happens in `Scene::set_follow_target`.

### Context (`context.hpp`)

Plain struct of raw pointers for callbacks. Lifetime is owned by `AppObjects`.

---

## Scene and camera (`src/app/scene/`)

### Scene (`scene.*`)

Owns:

- `BodyVisual` list
- `IRenderer`
- View half-extent (zoom), body-scaling flag
- Camera center in AU (`view_center_*`)
- Optional follow target + pan offset while following

Each `render` call:

1. If following, set `view_center` from body position (km → AU) + offset
2. Build `ViewFrame` (half extent, aspect, framebuffer height, scaling, center)
3. Each `BodyVisual::append_draw` fills a `DrawBatch`
4. `renderer_->draw(batch)`

**Pan:** free camera moves `view_center`; while following, moves
`follow_offset_*` instead. **Home:** clears follow, offset, and centers on the
Sun (origin).

### ViewFrame (`view_frame.hpp`)

Per-frame snapshot of camera / display parameters passed into visuals. Bodies
do not store view state.

### BodyVisual (`body_visual.*`)

One drawable body: name, color, radius, tail duration, display size factor,
whether orbit decorations are enabled (`draws_orbit_trails_`, set from
`semi_major_axis_km > 0` at construction).

`append_draw`:

1. Always sample `simulation.state(name)` and project via `to_point_instance`
   (view center offset applied here — same path for Sun and planets)
2. If orbital trails: grey closed orbit loop + fading tail line/points

Point size is derived from physical radius, zoom, framebuffer height, and
optional `display_size_factor`. OpenGL may clamp very large point sizes.

### Catalog and presentation config

| File | Role |
|------|------|
| `body_visual_catalog.*` | Walk catalog, merge defaults/overrides, `add_body` if visible |
| `body_visual_loader.*` | Parse `body_visuals.json` |
| `body_visual_config.hpp` | Defaults + per-body optional overrides |

**When changing how a body looks:** prefer `body_visuals.json`. **When changing
physics:** `bodies.json` / core. **When changing draw logic for all bodies:**
`body_visual.cpp`.

---

## Rendering (`src/app/render/`)

### Types (`types.hpp`)

CPU-side draw data: `PointInstance`, `LineVertex`, `LinePrimitive`, `DrawBatch`
(points, line strips for tails, line loops for orbits).

### IRenderer (`renderer.hpp`)

`init(RenderCapacity)` then `draw(DrawBatch)`. Capacity is sized from body
count and trail/orbit sample counts in `Scene::init`.

### GlRenderer (`gl_renderer.*`, `gl_shader.*`)

OpenGL 4.6: batched points (circular discard in fragment shader) and line
strips/loops. World → NDC is already done in `BodyVisual`; the renderer only
uploads and draws.

**When adding 3D later:** extend `ViewFrame` / projection in scene visuals first;
keep the renderer as a thin consumer of NDC (or later clip-space) batches if
possible.

---

## Assets

| Path | Contents |
|------|----------|
| `assets/data/bodies.json` | Name, mu, radius, Kepler elements (Sun-outward order) |
| `assets/data/body_visuals.json` | Defaults + per-body color, tail days, size factor, visible |

Mismatch warnings (override without catalog entry, or catalog without
override) are logged from `body_visual_catalog.cpp`.

---

## Tests (`tests/`)

Catch2 tests under `tests/`, registered via CMake. Focus areas:

- Kepler / ephemeris helpers
- JSON body loading
- Types (epoch, displacement polar)
- Horizons reference samples (`tests/data/horizons_reference.json`)
- Color parsing helpers

Run:

```bash
ctest --test-dir build --output-on-failure
```

There is little automated coverage of GLFW/OpenGL or the full scene path;
those are exercised by running the app.

---

## Mental models worth keeping

1. **Camera moves in the world.** Arrows move the view center (or follow
   offset). Objects appear to move the opposite way. Follow is
   `body position + offset` each frame.
2. **Names are the IDs.** Bodies are identified by string name across JSON,
   ephemeris, visuals, and follow targets.
3. **Sun is not a special draw path.** It is a body with `a_km == 0`, zero
   state from the ephemeris, and `draws_orbit_trails == false`.
4. **2D now, 3D-aware layout.** Positions are 3D km vectors; the view uses the
   ecliptic plane (`x`, `y`) and a 2D center. Prefer world-space camera state
   over screen-space hacks when extending.

---

## Suggested reading order for a first change

1. `README.md` controls and data files  
2. `src/app/run.cpp` — how a frame is built  
3. `src/app/scene/scene.cpp` + `body_visual.cpp` — camera and draw  
4. `src/sim/solar_system.cpp` + `src/core/kepler_ephemeris.cpp` — where state comes from  
5. `src/app/input.cpp` — how keys reach scene / clock  

For a **new body:** add to `bodies.json` and `body_visuals.json`, extend
`kFollowTargets` if it should be key-selectable, rebuild (assets copy
automatically).
