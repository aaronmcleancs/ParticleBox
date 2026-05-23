#ifndef FORCES_H
#define FORCES_H

#include "input_state.h"
#include "particle.h"

#include <cstddef>

// ---------------------------------------------------------------------------
// Field forces applied during the force-accumulation phase of the physics
// step. Each function operates on a contiguous [begin,end) range and only
// writes to per-particle data owned by that range, so they're safe to call
// in parallel.
// ---------------------------------------------------------------------------

namespace forces {

// Reset accX/accY for the range.
void zeroAccelerations(ParticleSystem &p, std::size_t begin, std::size_t end);

// Constant world gravity. Gas gets buoyancy (negative multiplier),
// stone gets nothing (it's kinematic).
void applyGravity(ParticleSystem &p, const InputState &in,
                  std::size_t begin, std::size_t end);

// WASD-driven wind: a flat additive acceleration affecting all non-stone.
void applyWind(ParticleSystem &p, const InputState &in,
               std::size_t begin, std::size_t end);

// Mouse-driven field: attract / repel / vortex, depending on InputState.mode.
// Only active while the left button is held (or always for vortex if you
// want; here it's gated on leftDown).
void applyMouseField(ParticleSystem &p, const InputState &in,
                     std::size_t begin, std::size_t end);

// One-shot radial impulse, written directly to velocity, then the flag is
// cleared by the caller. Safe to parallelise across the particle range.
void applyExplosionImpulse(ParticleSystem &p, const InputState &in,
                           std::size_t begin, std::size_t end);

// Per-type velocity damping (air resistance / liquid viscosity).
void applyDamping(ParticleSystem &p, std::size_t begin, std::size_t end);

} // namespace forces

#endif
