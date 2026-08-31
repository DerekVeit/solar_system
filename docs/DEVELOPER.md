# Developer guide

Orientation for someone new to this repository: how the pieces fit together,
where to look when changing behavior, and a few conventions that matter.

## High-level architecture

```
main → make_app / run_loop
         │
         ├─ Window (GLFW) + input callbacks
         ├─ SolarSystem (clock + EphemerisProvider)
         └─ Scene (BodyVisuals + sky + camera + IRenderer)
                    │
                    └─ each frame:
                         follow → ViewFrame
                         BodyVisual::append_draw
                         sky figures + star markers
                         GlRenderer: sky → spheres → rings → lines
```

Three layers keep physics, presentation, and windowing separate:

| Layer | Namespace | Responsibility |
|-------|-----------|----------------|
| **Core** | `solar::core` | Bodies, epochs, Kepler propagation, orientation, ephemeris interface |
| **Simulation** | `solar::sim` | Clock / time scale and a thin facade over the ephemeris |
| **App** | `solar::app` | Window, input, scene graph visuals, OpenGL drawing |

The simulation does not know about OpenGL. Rendering never mutates orbital
elements; it only samples positions and orientation at the current (or
historical) epoch.

Draw geometry is **camera-relative km** (world minus eye). The view matrix is
rotation-only (`lookAt` from the origin). `GlRenderer` applies view and
perspective projection; the scene does not write NDC.

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
    scene/        # camera + body visuals + sky + draw batching
    render/       # IRenderer, GlRenderer, GPU types
assets/data/      # bodies.json, body_visuals.json, sky.json, stars.json, constellations.json
assets/textures/  # globe / ring / star-map images
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
`StateVector` (glm `dvec3` in km and km/s), `KeplerianElements`, `BodyPole` /
`BodyRotation` (IAU WGCCRE-style), polar helpers.

### Constants (`constants.hpp`)

AU, day length, angle conversions, J2000 JD, etc.

### Kepler (`kepler.hpp` / `kepler.cpp`)

Mean motion, period, Kepler equation solver, state from elements at an epoch,
helpers used for orbit sampling (`epoch_before_mean_anomaly`,
`mean_anomaly_at_epoch`, …). Motion tails sample uniformly in time so a
duration longer than one period (e.g. the Moon) still traces the real path.
Inclination and node are applied; orbits are 3D in the ecliptic frame, not
forced into the xy-plane.

### Body orientation (`body_orientation.*`)

IAU pole (α₀, δ₀) plus prime meridian W(t) → body-fixed to ecliptic draw-frame
matrix (`Rx(−ε) · R_ICRF`). Tidally locked satellites replace catalog Ẇ with
Keplerian mean motion (sign of the catalog Ẇ kept) so the prime meridian stays
facing the primary on an osculating ellipse.

### Ephemeris interface (`ephemeris.hpp` / `ephemeris.cpp`)

- `BodyDefinition` — name, optional `primary` (empty = Sun), mu, radius, pole,
  rotation, `tidally_locked`, elements
- `EphemerisProvider` — `state(name, epoch)` (heliocentric), `relative_state`
  (Kepler about the primary), `rotation_deg`, `bodies()`
- `find_body`, `primary_body`, `orbital_mu` (primary’s mu),
  `central_gravitational_parameter` (Sun’s mu)
- `bodies_orbiting`, `innermost_satellite`, `sibling_by_offset` — catalog
  families sorted by semi-major axis (empty/`Sun` = heliocentric set)

### Kepler ephemeris (`kepler_ephemeris.*`)

Concrete provider: Kepler about the primary (`orbital_mu`), then compose
`state = state(primary) + relative`. The Sun (or any body with `a_km <= 0`)
is at the origin. The constructor rejects unknown, self, or cyclic primaries.
`rotation_deg` delegates to `rotation_deg_at_epoch`.

### JSON loader (`json_loader.*`)

Loads `assets/data/bodies.json` into `std::vector<BodyDefinition>`.
Optional `"primary"` names the attracting body; omit it for heliocentric
orbits. Satellite `kepler` blocks use the same inertial (ecliptic) frame as
the planets, with the focus at the primary — not the primary’s equator.
Also loads `pole`, `rotation`, and `tidally_locked`.

**When changing orbits or adding bodies:** update `bodies.json` and tests that
assert catalog contents; keep Sun first with a valid mu (required by
heliocentric Kepler). **When adding a moon:** set `"primary"` to the planet
name and give planetocentric ecliptic elements. Tidally locked satellites
need `"tidally_locked": true` so Ẇ uses the same Keplerian mean motion as
the orbit (IAU mean Ẇ will drift against an osculating ellipse). The sign
of the catalog Ẇ is kept so retrograde moons (Miranda, Triton, …) stay
locked in the right sense.

### Sky orientation (`sky_orientation.*`)

Galactic-from-equatorial and `tex_from_ecliptic`: sample directions for the
star map. The SSS equirectangular maps are galactic (Milky Way along the
texture equator) with the galactic centre at u = 0.5 and the north galactic
pole at v = 1 — IAU galactic followed by Ry(180°). Named-star reticles
(`stars.json`) use `ecliptic_direction` and `directional_circle` so a marker
sits on the same view ray as the sky. Constellation edges use
`inset_great_circle` so stick-figure lines stop short of each star.

---

## Simulation (`src/sim/`)

### Clock (`clock.*`)

`SimulationClock` holds the current `Epoch` and a `TimeScale`:

- `paused` — no advance
- `real_time` — 1 sim second per wall second
- `accelerated` — wall delta × `acceleration_`

`make_app` starts in **real time** with `acceleration_ == 1`. **T** selects
the accelerated scale; **-** / **+** then change `acceleration_`.
`advance(delta_seconds)` is called once per frame from the main loop.

### Solar system (`solar_system.*`)

Owns the ephemeris and clock. `state(body_name)` is shorthand for
`ephemeris.state(name, clock.epoch())`. `orientation(body_name)` is the
body-fixed → ecliptic matrix at the clock epoch (`body_orientation_matrix`
with `orbital_mu`).

**When changing time behavior:** clock only. **When changing how positions are
computed:** ephemeris / Kepler, not the clock. **When changing how globes
spin:** `body_orientation` / catalog pole and W.

---

## App: lifecycle (`src/app/`)

### Entry (`main.cpp`, `run.*`)

1. `make_app()` — load bodies, build ephemeris + clock, populate scene from
   visuals JSON, load `sky.json`, `stars.json`, and `constellations.json`,
   create window, register keys, `scene.init()`
2. `run_loop()` — advance clock, clear, `scene.render(...)`, swap, poll
3. `log_shutdown_report()` — sample positions / epoch on exit

### Window (`window.*`)

GLFW setup, user pointer, key callback hook, framebuffer size, clear color,
depth test, and alpha blending (trails, rings, sky figures).

### Input (`input.*`, `follow_targets.hpp`)

GLFW key callback via `AppContext` (window / simulation / scene pointers).
Follow targets `0`–`9` are a fixed name table (Sun through Pluto). **M**
follows the innermost satellite of the current body (Mercury from the Sun,
Io from Jupiter). **`,`** / **`.`** cycle siblings — bodies that share a
primary, sorted by semi-major axis (`bodies_orbiting`). **Insert** frames the
followed body using the current FOV (`radius = R / sin(fov / 2)`).
Validation happens in `Scene::set_follow_target`.

### Context (`context.hpp`)

Plain struct of raw pointers for callbacks. Lifetime is owned by `AppObjects`.

---

## Scene and camera (`src/app/scene/`)

### Scene (`scene.*`)

Owns:

- `BodyVisual` list
- `IRenderer`
- `Camera` (look-at target, yaw / pitch, radius)
- Body-scaling and graticule flags
- Optional follow body name + pan offset while following
- Sky spec, named-star catalog, and constellation / asterism stick figures

Each `render` call:

1. If following, set camera look-at from the body’s **drawn** position
   (km → AU) + offset. Drawn position matches `BodyVisual` (scaled satellite
   orbits when body scaling is on).
2. Build `ViewFrame` (camera snapshot, framebuffer height, scaling)
3. Each `BodyVisual::append_draw` fills a `DrawBatch`
4. `append_sky_figures` then `append_star_markers` (eye-relative sphere just
   inside the far plane)
5. `renderer_->draw(batch, view, projection)` — sky, then spheres, rings, lines

**Pan:** free camera pans the look-at target; while following, moves
`follow_offset_*` instead. Pan step is a fraction of `view_width_au` /
`view_height_au`, which still come from leftover orthographic
`half_extent_au_` (2 AU), not from the perspective frustum.
**Unfollow (grave accent):** leaves the look-at where it is.
**Home:** clears follow and offset, resets camera to default (origin look-at,
default yaw / pitch / radius).
**End:** resets yaw / pitch / radius; keeps the look-at.

### Camera (`camera.*`)

Orbit look-at: eye = target − radius × forward(yaw, pitch). Perspective FOV
is 45°, far plane 150 AU. Geometry handed to the renderer is eye-relative km;
`view_matrix()` is rotation-only.

### ViewFrame (`view_frame.hpp`)

Per-frame snapshot of camera / display parameters passed into visuals. Bodies
do not store view state.

### BodyVisual (`body_visual.*`)

One drawable body: name, `BodySurface` (color / ambient / emission / texture
paths), optional rings, radius, tail duration, display size factor, optional
satellite-orbit factor inherited from the primary, and whether orbit
decorations are enabled (`draws_orbit_trails_`, set from
`semi_major_axis_km > 0` in the constructor).

Km-space sampling lives in `body_visual_geometry.*` (no OpenGL): drawn
position, closed loop around a frozen primary, and a tail sampled uniformly
in time. `append_draw` maps those km samples into camera-relative
`DrawBatch` vertices.

1. Sample physical `simulation.state(name)` for lighting (direction to the
   Sun). Draw the sphere at `drawn_position`: for a moon, that is
   `state(primary) + f * (state(moon) − state(primary))`, where `f` is the
   primary’s `moon_orbit_display_size_factor` when body scaling is on
   (otherwise `f = 1`). Globe rotation is `simulation.orientation(name)`.
2. Optional ring instances share that center, rotation, and size factor.
3. If orbital trails: grey closed orbit loop around the **frozen** primary
   (so a lunar ellipse stays closed) + fading tail along the composed path.
   Both use the same `f`.

Drawn radius is derived from physical radius and optional
`display_size_factor` (body size scaling can be toggled with `B` /
`Shift+B`). Planet size factors and moon-orbit factors are independent: 500×
Earth would swallow a 1:1 lunar orbit, so Earth uses a modest moon-orbit
factor (~15) that clears the scaled Earth without reaching Venus or Mars.

### Sky figures and star markers

- `sky_loader.*` — `sky.json` (texture path, longitude offset, brightness)
- `star_catalog_loader.*` — HIP-keyed `stars.json`; empty `name` means no
  reticle or label; duplicate HIP is a load error
- `sky_figure_loader.*` — `constellations.json`; every HIP in a polyline must
  exist in the star catalog
- `sky_figure_lines.*` — one `GL_LINES` primitive per figure; edges inset by
  `line_gap_deg` (default 0.5°)
- `star_markers.*` / `stroke_font.*` — billboarded reticles and labels for
  named stars only; **C** / **X** hide the whole marker set via
  `catalog.visible`. Figures are independent and have no key yet.

Asterisms are a flat list next to constellations (`kind` field), not nested
under a parent. Overlap (Dipper on UMa, Great Square on Pegasus) is expected.

### Catalog and presentation config

| File | Role |
|------|------|
| `body_visual_catalog.*` | Walk catalog, resolve spec, construct `BodyVisual` if visible |
| `body_visual_loader.*` | Parse `body_visuals.json` into fully resolved specs |
| `body_visual_config.hpp` | `BodySurface`, `RingSpec`, `BodyVisualSpec`, `BodyVisualConfig` |

JSON shape:

- **`defaults`** — fully required: nested `surface` (`color`, `ambient`,
  `emission`; `textures` optional), plus `tail_duration_days`,
  `display_size_factor`, `visible`
- **`surface.textures`** — optional paths (`diffuse`, `night`, `clouds`,
  `normal`, `specular`) and `longitude_offset_deg`
- **`rings`** — optional array of `{ map, inner_radius_km, outer_radius_km }`
- **`moon_orbit_display_size_factor`** — optional on a **primary** (default
  1). Applied to every satellite of that body when scaling is on; moons do
  not set this on themselves.
- **`bodies`** — optional per-name rows; each field (including nested
  `surface` keys) merges onto defaults. Catalog bodies without a row use
  defaults as-is.

**When changing how a body looks:** prefer `body_visuals.json`. **When changing
physics:** `bodies.json` / core. **When changing where bodies are drawn:**
`body_visual_geometry.cpp`. **When changing GPU batching:** `body_visual.cpp`.

---

## Rendering (`src/app/render/`)

### Types (`types.hpp`)

CPU-side draw data: `SphereInstance`, `RingInstance`, `LineVertex`,
`LinePrimitive`, `DrawBatch` (spheres, rings, `GL_LINES` for figures/labels,
line strips for tails, line loops for orbits and reticles). Positions are
camera-relative km.

### IRenderer (`renderer.hpp`)

`init(RenderCapacity)` then `draw(DrawBatch, view, projection)`. Capacity is
sized from body count, trail/orbit samples, and the largest star-label or
figure primitive in `Scene::init`. Also `upload_texture` and `set_sky`.

### GlRenderer (`gl_renderer.*`, `gl_shader.*`, `gl_pipeline.*`, `mesh_gen.*`)

OpenGL 4.6: directional starfield (fullscreen triangle, no sky mesh), then
spheres, rings, and line strips/loops/segments. The sky samples an
equirectangular map from the view ray after `tex_from_ecliptic` (galactic
frame + longitude offset). The renderer is a thin consumer of camera-relative
batches plus the two camera matrices.

**When adding a new draw type:** add it to `DrawBatch`, size it in
`RenderCapacity` / `Scene::init`, and draw it in `GlRenderer` with the same
view/projection path. Keep world→eye conversion in the scene (or a helper),
not in the GPU layer.

---

## Assets

| Path | Contents |
|------|----------|
| `assets/data/bodies.json` | Name, optional primary, mu, radius, pole, rotation, Kepler elements, moons |
| `assets/data/body_visuals.json` | Defaults + per-body surface, textures, rings, tail days, size factor, moon orbit scale, visible |
| `assets/data/sky.json` | Star map path, galactic longitude offset, brightness |
| `assets/data/stars.json` | HIP-keyed J2000 RA/Dec stars; optional names draw reticles and labels |
| `assets/data/constellations.json` | 40 IAU showpiece constellations plus 7 asterisms as HIP polylines |
| `assets/textures/` | SSS globe / ring / sky maps; see `ATTRIBUTION.md` |

Mismatch warnings (visual entry with no catalog body, or catalog body with no
visual entry — the latter uses defaults) are logged from
`body_visual_catalog.cpp`.

---

## Tests (`tests/`)

Catch2 tests under `tests/`, registered via CMake. Focus areas:

- Kepler / ephemeris helpers
- JSON body loading
- Types (epoch, displacement polar)
- Body orientation (pole, W, tidal lock)
- Horizons reference samples (`tests/data/horizons_reference.json`)
- Body visual geometry and config merge
- Sky orientation, star-marker tessellation, constellation figure load/inset
- Color parsing helpers

Run:

```bash
ctest --test-dir build --output-on-failure
```

There is little automated coverage of GLFW/OpenGL or the full scene path;
those are exercised by running the app.

---

## Mental models worth keeping

1. **Orbit camera, eye-relative draw.** Arrows move the look-at (or follow
   offset). Eye = target − radius × forward. Batches are km relative to the
   eye; the view matrix does not translate. Follow is `drawn position +
   offset` each frame.
2. **Names are the IDs.** Bodies are identified by string name across JSON,
   ephemeris, visuals, and follow targets. Stars in figures are identified by
   HIP, not by display name.
3. **Sun is not a special draw path.** It is a body with `a_km == 0`, zero
   state from the ephemeris, and `draws_orbit_trails == false`.
4. **`state()` is heliocentric; drawing may stretch satellite orbits.**
   Physics stays in the ephemeris. Presentation (`f` on the primary→moon
   vector) lives in `BodyVisual`. Follow uses the drawn position.
5. **Draw frame is ecliptic J2000.** +X toward the vernal equinox, +Z north
   of the ecliptic. The star map is a directional lookup on that sphere, not
   a mesh sitting in the scene.

---

## Suggested reading order for a first change

1. `README.md` controls and data files
2. `src/app/run.cpp` — how a frame is built
3. `src/app/scene/scene.cpp` + `body_visual.cpp` — camera and draw
4. `src/sim/solar_system.cpp` + `src/core/kepler_ephemeris.cpp` — where state comes from
5. `src/app/input.cpp` — how keys reach scene / clock

For a **new planet:** add to `bodies.json` and `body_visuals.json`, extend
`kFollowTargets` if it should be key-selectable, rebuild (assets copy
automatically). **`,`** / **`.`** will pick it up from `bodies_orbiting`.

For a **new moon:** same files, plus `"primary"` on the moon and
`moon_orbit_display_size_factor` on the planet if the scaled primary would
swallow a 1:1 orbit. Do not put the orbit factor on `bodies.json`. Set
`tidally_locked` if the prime meridian should stay facing the planet.
Major moons use `tail_duration_days` 0 so heliocentric tails do not clutter
the solar-system view; grey orbit loops remain.
