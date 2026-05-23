#include "physics.h"

#include "collisions.h"
#include "forces.h"

#include <algorithm>
#include <cmath>

PhysicsEngine::PhysicsEngine()
    : pool_(0) // ThreadPool default-sizes to hardware concurrency
{
  hash_ = std::make_unique<SpatialHash>(cfg::WORLD_WIDTH, cfg::WORLD_HEIGHT,
                                        cfg::SPATIAL_CELL_SIZE);
}

std::size_t PhysicsEngine::chunkSize(std::size_t total) const {
  if (!multithreading_) return total;
  unsigned int n = pool_.size();
  if (n == 0) return total;
  // Aim for ~4 chunks per worker so unbalanced regions are still load
  // balanced by chunk stealing.
  std::size_t target = std::max<std::size_t>(cfg::MIN_PARTICLES_PER_THREAD,
                                             total / (n * 4 + 1));
  return std::max<std::size_t>(1, target);
}

void PhysicsEngine::runParallel(std::size_t total,
                                const ThreadPool::RangeFn &fn) {
  if (total == 0) return;
  if (!multithreading_) {
    fn(0, total);
    return;
  }
  pool_.parallelFor(total, chunkSize(total), fn);
}

void PhysicsEngine::update(ParticleSystem &particles, const InputState &input,
                           float frameDt) {
  consumedExplosion_ = false;
  if (particles.count == 0) return;

  const int   substeps = std::max(1, input.substeps);
  const float dt       = (frameDt * input.timeScale) / static_cast<float>(substeps);
  const std::size_t N  = particles.count;

  // Capture once for closures.
  ParticleSystem *pp = &particles;
  const InputState *in = &input;

  for (int s = 0; s < substeps; ++s) {

    // ----- Phase 1: field accelerations -----
    runParallel(N, [pp, in](std::size_t b, std::size_t e) {
      forces::zeroAccelerations(*pp, b, e);
      forces::applyGravity     (*pp, *in, b, e);
      forces::applyWind        (*pp, *in, b, e);
      forces::applyMouseField  (*pp, *in, b, e);
    });

    // ----- Phase 2: integrate velocity + damping + one-shot impulse -----
    runParallel(N, [pp, in, dt](std::size_t b, std::size_t e) {
      auto &p = *pp;
      for (std::size_t i = b; i < e; ++i) {
        if (p.type[i] == TYPE_STONE) continue;
        p.velX[i] += p.accX[i] * dt;
        p.velY[i] += p.accY[i] * dt;
      }
      forces::applyDamping(p, b, e);
      forces::applyExplosionImpulse(p, *in, b, e);
    });

    // ----- Phase 3: integrate position -----
    runParallel(N, [pp, dt](std::size_t b, std::size_t e) {
      auto &p = *pp;
      for (std::size_t i = b; i < e; ++i) {
        if (p.type[i] == TYPE_STONE) continue;
        p.posX[i] += p.velX[i] * dt;
        p.posY[i] += p.velY[i] * dt;
      }
    });

    // Mark explosion as consumed for this frame (the first substep applied it).
    if (s == 0 && input.explodePending) {
      consumedExplosion_ = true;
    }

    // ----- Phase 4: rebuild spatial hash (serial; counting-sort O(N)) -----
    if (gridEnabled_) {
      hash_->build(sortedIndices_, particles.posX, particles.posY, N);
    }

    // ----- Phase 5: collision corrections (Jacobi-style) -----
    if (gridEnabled_) {
      runParallel(N, [pp, this](std::size_t b, std::size_t e) {
        collisions::resolveBand(*pp, *hash_, sortedIndices_, b, e);
      });

      // ----- Phase 6: apply scratch corrections + world bounds -----
      runParallel(N, [pp](std::size_t b, std::size_t e) {
        auto &p = *pp;
        for (std::size_t i = b; i < e; ++i) {
          if (p.type[i] == TYPE_STONE) continue;
          p.posX[i] += p.accX[i];
          p.posY[i] += p.accY[i];
        }
        collisions::applyWorldBounds(p, b, e);
      });
    } else {
      // Without spatial hash, just clip to world bounds.
      runParallel(N, [pp](std::size_t b, std::size_t e) {
        collisions::applyWorldBounds(*pp, b, e);
      });
    }
  }
}
