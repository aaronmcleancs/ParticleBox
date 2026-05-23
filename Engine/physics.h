#ifndef PHYSICS_H
#define PHYSICS_H

#include "input_state.h"
#include "particle.h"
#include "spatial_hash.h"
#include "thread_pool.h"

#include <cstdint>
#include <memory>
#include <vector>

// ---------------------------------------------------------------------------
// Orchestrates one physics frame:
//
//   for substep in 0..N:
//       1. accumulate field accelerations into accX/accY                (parallel)
//       2. integrate velocity from acc, damp, optional explosion impulse (parallel)
//       3. integrate position from velocity                              (parallel)
//       4. rebuild spatial hash                                          (serial)
//       5. collision detection -> per-particle position correction       (parallel)
//       6. apply correction + world bounds                               (parallel)
//
// Every step that writes per-particle state only writes the index it owns,
// so the parallel passes are race-free. The collision step uses accX/accY
// as a scratch buffer for position corrections (the field acceleration is
// no longer needed by the time we reach the collision phase).
// ---------------------------------------------------------------------------

class PhysicsEngine {
public:
  PhysicsEngine();

  void update(ParticleSystem &particles, const InputState &input,
              float frameDt);

  void setMultithreadingEnabled(bool b) { multithreading_ = b; }
  void setGridEnabled(bool b)           { gridEnabled_ = b; }

  // Lets the caller clear the one-shot explode flag after consumption.
  bool consumedExplosionFlag() const { return consumedExplosion_; }

private:
  bool multithreading_ = true;
  bool gridEnabled_    = true;
  bool consumedExplosion_ = false;

  ThreadPool                pool_;
  std::unique_ptr<SpatialHash> hash_;
  std::vector<std::uint32_t>   sortedIndices_;

  std::size_t chunkSize(std::size_t total) const;
  void runParallel(std::size_t total,
                   const ThreadPool::RangeFn &fn);
};

#endif
