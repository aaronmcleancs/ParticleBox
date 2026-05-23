#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include "config.h"
#include "particle.h"
#include "vec2.h"

// ---------------------------------------------------------------------------
// Plain-old-data container shared between the input layer, the physics, and
// the HUD/help overlay. Keeping it as a struct (no behaviour) means the
// physics engine can read it lock-free during its update step while the UI
// thread writes to it from event handlers.
// ---------------------------------------------------------------------------

enum class MouseMode : int {
  Spawn = 0,
  Attract,
  Repel,
  Vortex,
  Drag,
  Explode,
  Erase,
  Count
};

const char *mouseModeName(MouseMode m);

struct InputState {
  // Simulation control
  bool  paused    = false;
  float timeScale = 1.0f;
  int   substeps  = cfg::PHYSICS_SUBSTEPS;

  // Mouse
  Vec2      mousePos       {0.0f, 0.0f};
  bool      leftDown       = false;
  bool      rightDown      = false;
  MouseMode mode           = MouseMode::Spawn;
  float     brushRadius    = 60.0f;
  int       spawnPerTick   = 1;
  ParticleType spawnType   = TYPE_DEFAULT;

  // One-shot triggers consumed by the physics engine on the next step.
  bool explodePending      = false;
  Vec2 explodePosition     {0.0f, 0.0f};

  // Global force field
  Vec2  gravity            {0.0f, 9.81f};
  bool  gravityEnabled     = true;
  Vec2  wind               {0.0f, 0.0f}; // refreshed each frame from WASD state

  // Engine flags
  bool gridEnabled         = true;
  bool multithreadEnabled  = true;

  // HUD
  bool showHelp            = true;
};

#endif
