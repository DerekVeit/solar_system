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

- `BodyDefinition` — name, optional `primary` (empty = Sun), mu, radius, elements
- `EphemerisProvider` — `state(name, epoch)` (heliocentric), `relative_state`
  (Kepler about the primary), `bodies()`
- `find_body`, `primary_body`, `orbital_mu` (primary’s mu),
  `central_gravitational_parameter` (Sun’s mu)

### Kepler ephemeris (`kepler_ephemeris.*`)

Concrete provider: Kepler about the primary (`orbital_mu`), then compose
`state = state(primary) + relative`. The Sun (or any body with `a_km <= 0`)
is at the origin. The constructor rejects unknown, self, or cyclic primaries.

### JSON loader (`json_loader.*`)

Loads `assets/data/bodies.json` into `std::vector<BodyDefinition>`.
Optional `"primary"` names the attracting body; omit it for heliocentric
orbits. Satellite `kepler` blocks use the same inertial (ecliptic) frame as
the planets, with the focus at the primary — not the primary’s equator.

**When changing orbits or adding bodies:** update `bodies.json` and tests that
assert catalog contents; keep Sun first with a valid mu (required by
heliocentric Kepler). **When adding a moon:** set `"primary"` to the planet
name and give planetocentric ecliptic elements.

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
Follow targets `0`–`9` are a fixed name table (Sun through Pluto); **M**
follows the Moon. Validation happens in `Scene::set_follow_target`.

### Context (`context.hpp`)

Plain struct of raw pointers for callbacks. Lifetime is owned by `AppObjects`.

---

## Scene and camera (`src/app/scene/`)

### Scene (`scene.*`)

Owns:

- `BodyVisual` list
- `IRenderer`
- `Camera` (look-at target, orbit angles, radius / framing)
- Body-scaling flag
- Optional follow body name + pan offset while following

Each `render` call:

1. If following, set camera look-at from the body’s **drawn** position
   (km → AU) + offset. Drawn position matches `BodyVisual` (scaled satellite
   orbits when body scaling is on).
2. Build `ViewFrame` (camera snapshot, framebuffer height, scaling)
3. Each `BodyVisual::append_draw` fills a `DrawBatch`
4. `renderer_->draw(batch, view, projection)`

**Pan:** free camera pans the look-at target; while following, moves
`follow_offset_*` instead. **Unfollow:** leaves the look-at where it is.
**Home:** clears follow and offset, resets camera to default (origin look-at).

### ViewFrame (`view_frame.hpp`)

Per-frame snapshot of camera / display parameters passed into visuals. Bodies
do not store view state.

### BodyVisual (`body_visual.*`)

One drawable body: name, `BodySurface` (color / ambient / emission), radius,
tail duration, display size factor, optional satellite-orbit factor inherited
from the primary, and whether orbit decorations are enabled
(`draws_orbit_trails_`, set from `semi_major_axis_km > 0` in the constructor).

`append_draw`:

1. Sample physical `simulation.state(name)` for lighting (direction to the
   Sun). Draw the sphere at `drawn_position`: for a moon, that is
   `state(primary) + f * (state(moon) − state(primary))`, where `f` is the
   primary’s `moon_orbit_display_size_factor` when body scaling is on
   (otherwise `f = 1`).
2. If orbital trails: grey closed orbit loop around the **frozen** primary
   (so a lunar ellipse stays closed) + fading tail along the composed path.
   Both use `orbital_mu` and the same `f`.

Drawn radius is derived from physical radius and optional
`display_size_factor` (body size scaling can be toggled with `B` /
`Shift+B`). Planet size factors and moon-orbit factors are independent: 500×
Earth would swallow a 1:1 lunar orbit, so Earth uses a modest moon-orbit
factor (~15) that clears the scaled Earth without reaching Venus or Mars.

### Catalog and presentation config

| File | Role |
|------|------|
| `body_visual_catalog.*` | Walk catalog, resolve spec, construct `BodyVisual` if visible |
| `body_visual_loader.*` | Parse `body_visuals.json` into fully resolved specs |
| `body_visual_config.hpp` | `BodySurface`, `BodyVisualSpec`, `BodyVisualConfig` |

JSON shape:

- **`defaults`** — fully required: nested `surface` (`color`, `ambient`,
  `emission`), plus `tail_duration_days`, `display_size_factor`, `visible`
- **`moon_orbit_display_size_factor`** — optional on a **primary** (default
  1). Applied to every satellite of that body when scaling is on; moons do
  not set this on themselves.
- **`bodies`** — optional per-name rows; each field (including nested
  `surface` keys) merges onto defaults. Catalog bodies without a row use
  defaults as-is.

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
| `assets/data/bodies.json` | Name, optional primary, mu, radius, Kepler elements |
| `assets/data/body_visuals.json` | Defaults + per-body surface, tail days, size factor, moon orbit scale, visible |

Mismatch warnings (visual entry with no catalog body, or catalog body with no
visual entry — the latter uses defaults) are logged from
`body_visual_catalog.cpp`.

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
   `drawn position + offset` each frame.
2. **Names are the IDs.** Bodies are identified by string name across JSON,
   ephemeris, visuals, and follow targets.
3. **Sun is not a special draw path.** It is a body with `a_km == 0`, zero
   state from the ephemeris, and `draws_orbit_trails == false`.
4. **`state()` is heliocentric; drawing may stretch satellite orbits.**
   Physics stays in the ephemeris. Presentation (`f` on the primary→moon
   vector) lives in `BodyVisual`. Follow uses the drawn position.
5. **2D now, 3D-aware layout.** Positions are 3D km vectors; the view uses the
   ecliptic plane (`x`, `y`) and a 2D center. Prefer world-space camera state
   over screen-space hacks when extending.

---

## Suggested reading order for a first change

1. `README.md` controls and data files  
2. `src/app/run.cpp` — how a frame is built  
3. `src/app/scene/scene.cpp` + `body_visual.cpp` — camera and draw  
4. `src/sim/solar_system.cpp` + `src/core/kepler_ephemeris.cpp` — where state comes from  
5. `src/app/input.cpp` — how keys reach scene / clock  

For a **new planet:** add to `bodies.json` and `body_visuals.json`, extend
`kFollowTargets` if it should be key-selectable, rebuild (assets copy
automatically).

For a **new moon:** same files, plus `"primary"` on the moon and
`moon_orbit_display_size_factor` on the planet if the scaled primary would
swallow a 1:1 orbit. Do not put the orbit factor on `bodies.json`.
