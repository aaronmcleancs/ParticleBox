#ifndef CONFIG_H
#define CONFIG_H

// ---------------------------------------------------------------------------
// Global compile-time constants for the particle simulation.
// Centralising every magic number here makes the rest of the engine read
// cleanly and lets us tweak behaviour without hunting through source files.
// ---------------------------------------------------------------------------

namespace cfg {

// World / window
constexpr float WORLD_WIDTH  = 1200.0f;
constexpr float WORLD_HEIGHT = 800.0f;

// Spatial hash: cell size must be >= 2 * max particle radius so a particle
// only ever has to look at the 3x3 cells around it. We use a power of two so
// the build step can shift instead of dividing.
constexpr float SPATIAL_CELL_SIZE = 8.0f;
constexpr int   SPATIAL_CELL_SHIFT = 3; // log2(8)

// Integration
constexpr int   PHYSICS_SUBSTEPS = 4;   // sub-steps per render frame
constexpr float DT_DEFAULT       = 0.10f;

// Boundaries
constexpr float BOUNDARY_RESTITUTION = 0.85f;

// Particle defaults
constexpr float DEFAULT_RADIUS = 2.5f;
constexpr float DEFAULT_MASS   = 1.0f;
constexpr int   INITIAL_CAPACITY = 16384;

// Collision response (PBD-style: positions are projected out of overlap,
// then an impulse exchange handles velocity).
constexpr float COLLISION_RESTITUTION = 0.40f; // 0=perfectly inelastic, 1=elastic
constexpr float COLLISION_FRICTION    = 0.10f;
constexpr float POSITION_BIAS         = 0.50f; // share of overlap each particle takes

// Mouse interaction strengths
constexpr float MOUSE_REPEL_STRENGTH  = 1800.0f;
constexpr float MOUSE_ATTRACT_STRENGTH = 1200.0f;
constexpr float MOUSE_VORTEX_STRENGTH = 1200.0f;
constexpr float MOUSE_EXPLODE_IMPULSE = 600.0f;
constexpr float MOUSE_DRAG_STIFFNESS  = 18.0f;

// Wind / directional forces triggered via WASD
constexpr float WIND_FORCE = 80.0f;

// Drag / damping
constexpr float GAS_DAMPING    = 0.985f;
constexpr float LIQUID_DAMPING = 0.998f;
constexpr float DEFAULT_DAMPING = 0.9995f;

// Type-specific behaviour
constexpr float LIQUID_COHESION    = 0.45f; // multiplies overlap correction
constexpr float SAND_FRICTION_COEF = 0.20f;
constexpr float GAS_BUOYANCY_MULT  = -0.20f; // multiplier on gravity for gas

// Rendering
constexpr int RENDER_CIRCLE_VERTS = 12; // polygon edges per particle

// Threading
constexpr int  MIN_PARTICLES_PER_THREAD = 256;

} // namespace cfg

#endif
