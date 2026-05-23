# ParticleSimulation

A C++/SDL2 particle simulator featuring multithreaded physics with sub-stepped
integration, a uniform spatial hash for collision broadphase, batched GPU
rendering, and a rich set of interactive keyboard and mouse controls.

This is a substantial restructure of the original project. Highlights:

- **Real iterative collision resolution** with mass-weighted positional
  correction, restitution, friction and per-type behaviour (sand, liquid,
  gas, stone, default).
- **Sub-stepped integration** (default 4 substeps/frame) for stability at
  high densities and large impulses.
- **Persistent thread pool** with parallel-for over particle ranges, replacing
  per-frame `std::async` spawns.
- **Batched rendering** via a single `SDL_RenderGeometry` call per frame
  instead of `SDL_RenderCopy` per particle - this alone removed the previous
  hard ceiling around a few thousand particles.
- **Race-free updates**: each worker thread writes only to its own particle
  indices and reads neighbours' state through scratch buffers, eliminating
  the data race in the original `updateChunk`.
- **Rich interactive controls** (see below): seven mouse modes, five particle
  types, wind, tiltable gravity, brush sizing, explosions, freeze, clear,
  reset, pause, time scaling, multi-thread/grid/help toggles, and more.
- **Cross-platform** Makefile (macOS / Linux / MinGW). No more hardcoded
  font paths - fonts are discovered automatically or via `PARTICLE_FONT`.

---

## Project Layout

```
ParticleSimulation/
├── main.cpp
├── Makefile
├── Engine/
│   ├── config.h           Central simulation tunables
│   ├── vec2.h             Small 2D vector type
│   ├── particle.{h,cpp}   SoA particle system + ParticleType
│   ├── spatial_hash.h     Uniform-grid broadphase
│   ├── input_state.h      Shared input/runtime state
│   ├── input_manager.{h,cpp}  SDL event -> InputState
│   ├── forces.{h,cpp}     Gravity / wind / mouse field / explosions / damping
│   ├── collisions.{h,cpp} Jacobi-style positional + velocity resolution
│   ├── physics.{h,cpp}    PhysicsEngine: orchestrates substeps & phases
│   ├── thread_pool.{h,cpp}    Persistent worker pool + parallelFor
│   ├── simulation.{h,cpp} Top-level Simulation facade
│   └── test.{h,cpp}       Headless benchmark suite (-test)
└── UI/
    ├── particle_renderer.{h,cpp}  Batched SDL_RenderGeometry
    ├── help_overlay.{h,cpp}       Status bar + keymap overlay
    ├── gui.{h,cpp}                Side-panel control window
    └── font_finder.{h,cpp}        Cross-platform font lookup
```

---

## Building

### Linux

```
sudo apt-get install build-essential libsdl2-dev libsdl2-ttf-dev
make
./ParticleSimulator
```

### macOS

```
brew install sdl2 sdl2_ttf
make
./ParticleSimulator
```

### Windows (MSYS2 / MinGW)

```
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
make
./ParticleSimulator.exe
```

### Targets

| Command       | Effect                                       |
|---------------|----------------------------------------------|
| `make`        | Optimised build                              |
| `make debug`  | `-O0 -g` for use with gdb / lldb             |
| `make test`   | Builds and runs headless benchmark           |
| `make clean`  | Remove all build artefacts                   |
| `make help`   | Print available targets                      |

---

## Fonts

The status bar and help overlay use a TTF font, but the simulation itself
will run fine without one. The font is searched in this order:

1. The path in the `PARTICLE_FONT` environment variable, if set.
2. A built-in list of standard system fonts (DejaVuSans on Linux, Menlo or
   Arial on macOS, Consolas / Arial on Windows).

If nothing is found, the on-screen text overlays are silently disabled.

```
PARTICLE_FONT=/path/to/your.ttf ./ParticleSimulator
```

---

## Controls

### Global

| Key           | Action                                          |
|---------------|-------------------------------------------------|
| **Space**     | Pause / resume simulation                       |
| **R**         | Reset world (re-seeds 1000 random particles)    |
| **C**         | Clear all particles                             |
| **F**         | Freeze - zero every particle's velocity         |
| **G**         | Toggle gravity                                  |
| **M**         | Toggle multithreading                           |
| **B**         | Toggle spatial-grid broadphase                  |
| **H**         | Toggle keymap overlay                           |
| **Escape**    | Quit                                            |

### Particle Type Selection

| Key | Type     | Notes                                              |
|-----|----------|----------------------------------------------------|
| 1   | Default  | Balanced mass, mild damping                        |
| 2   | Liquid   | Lower friction, slightly lighter, flows            |
| 3   | Sand     | High friction, settles into piles                  |
| 4   | Gas      | Buoyant - rises against gravity                    |
| 5   | Stone    | Effectively immovable (infinite mass)              |

### Mouse Modes

Cycle with **Q** (previous) / **E** (next). Current mode is shown in the
status bar and as the colour of the brush ring.

| Mode     | Left click / drag effect                                 |
|----------|----------------------------------------------------------|
| Spawn    | Continuously emit particles of the selected type         |
| Attract  | Pull particles toward the cursor                         |
| Repel    | Push particles away from the cursor                      |
| Vortex   | Swirl particles around the cursor                        |
| Drag     | Damp velocities inside the brush radius                  |
| Explode  | Single-tap radial impulse - releases on mouse down       |
| Erase    | Delete particles inside the brush                        |

Mouse wheel adjusts brush radius. Right-click is always repel, regardless
of the active mode.

### Wind (held)

| Key | Direction |
|-----|-----------|
| W   | Up        |
| A   | Left      |
| S   | Down      |
| D   | Right     |

Wind dies off as soon as the keys are released.

### Gravity Tweaks

| Key            | Effect                              |
|----------------|-------------------------------------|
| Arrow Up       | Reduce gravity Y                    |
| Arrow Down     | Increase gravity Y                  |
| Arrow Left     | Tilt gravity left                   |
| Arrow Right    | Tilt gravity right                  |
| `[`            | Step gravity Y down                 |
| `]`            | Step gravity Y up                   |

### Time Scaling

| Key   | Effect                                |
|-------|---------------------------------------|
| `-`   | Slow simulation time                  |
| `=`   | Speed simulation time up              |
| `0`   | Reset time scale to 1.0               |

### Spawn Brush Tuning

| Key       | Effect                                  |
|-----------|-----------------------------------------|
| Page Up   | Increase particles per tick (spawn)     |
| Page Down | Decrease particles per tick (spawn)     |

---

## GUI Control Window

A second window provides buttons and a live FPS / count / average velocity
graph. Every action available in the GUI also has a key binding, so the GUI
is optional for power users.

---

## Benchmark

```
make test
```

Runs a headless benchmark across several particle counts, comparing
single-threaded vs multi-threaded execution and with vs without the
spatial grid. Disabling the grid skips pairwise collision resolution
entirely, so those numbers are integration-only - useful as a baseline
for how much time the broadphase is consuming.

---

## Architecture Notes

**Simulation step** (per substep):

1. `forces.zeroAccelerations()` - reset per-particle acceleration buffers.
2. `forces.applyGravity / applyWind / applyMouseField / applyExplosion` -
   accumulate body forces.
3. Integrate velocity (`v += a * dt`).
4. Integrate position (`p += v * dt`).
5. Rebuild spatial hash.
6. `collisions.resolveBand` - per-cell Jacobi position correction with
   mass-weighted impulse and per-type friction. Uses the now-free
   acceleration buffers as scratch space - no extra allocation.
7. `collisions.applyWorldBounds` - clamp to world rect with restitution.

Threads write only to their own particle index `i` and read other indices
through const refs, so the update is data-race-free without locks.

**Rendering**: every particle becomes a small triangle fan (12 verts) added
to a single vertex buffer; one `SDL_RenderGeometry` call draws every
particle. Colour is interpolated from the particle's base colour toward
hot orange based on speed squared - so fast particles glow.

---

## Credits

Originally written in C++/SDL2; restructured here for performance, physics
correctness, and interactivity.
