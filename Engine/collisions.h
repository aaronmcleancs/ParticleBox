#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "particle.h"
#include "spatial_hash.h"

#include <cstdint>
#include <vector>

class ThreadPool;

// ---------------------------------------------------------------------------
// Two-phase collision resolution.
//
// Phase 1 — position project: resolve overlap by moving each pair half the
// penetration depth apart. Operates on positions only and is therefore
// commutative-ish; we apply it in a deterministic neighbour-walk order so
// the result is stable. To make it parallel-safe we operate on owner-only
// half-pair writes guarded by the (i<j) ordering: each thread updates its
// own index `i` but never writes to neighbours.
//
// Phase 2 — velocity impulse: at the end of all position corrections we
// recompute normal-direction velocity exchange with a restitution coefficient
// and apply tangential friction. Same single-writer rule.
//
// Both phases are split into 4 colours of an interleaved cell pattern to
// remove any read/write conflicts between threads while keeping work
// balanced.
// ---------------------------------------------------------------------------

namespace collisions {

// Resolve overlap by projecting each particle out by half the penetration
// depth, and exchange momentum along the contact normal. Safe to call in
// parallel within a single colour band.
void resolveBand(ParticleSystem &p,
                 const SpatialHash &hash,
                 const std::vector<std::uint32_t> &sortedIndices,
                 std::size_t begin, std::size_t end);

// Apply boundary collision (world walls) inline.
void applyWorldBounds(ParticleSystem &p, std::size_t begin, std::size_t end);

} // namespace collisions

#endif
