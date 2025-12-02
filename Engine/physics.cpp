#include "physics.h"
#include <algorithm>
#include <cmath>
#include <future>
#include <thread>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif

PhysicsEngine::PhysicsEngine() {
  // Initialize spatial hash with 1200x800 world and 8.0f cell size
  spatialHash = std::make_unique<SpatialHash>(1200.0f, 800.0f, 8.0f);
}

void PhysicsEngine::update(ParticleSystem &particles, float deltaTime) {
  if (particles.count == 0)
    return;

  // 1. Build Spatial Hash
  if (gridEnabled) {
    spatialHash->build(sortedIndices, particles.posX, particles.posY,
                       particles.count);
  }

  const float dt = deltaTime;
  const float dtSq = dt * dt;
  const size_t count = particles.count;

  // We'll use a simple parallel loop for now.
  // For M1, we can use dispatch_apply or std::thread.
  // Let's stick to a simple chunk-based approach compatible with C++17.

  auto updateChunk = [&](size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
      // --- Verlet Integration Step 1: Position Update ---
      // x(t+dt) = x(t) + v(t)dt + 0.5*a(t)dt^2
      // But we are storing v, so let's stick to Semi-Implicit Euler or Velocity
      // Verlet. Let's use Semi-Implicit Euler for simplicity and stability with
      // collisions: v += a * dt x += v            // Reset acceleration
      float ax = 0.0f;
      float ay = 0.0f;

      int type = particles.type[i];

      // Gravity & Buoyancy
      if (gravityEnabled) {
        if (type == TYPE_GAS) {
          ay -= gravity * 0.1f; // Slight buoyancy (slow rise)
        } else if (type != TYPE_STONE) {
          ay += gravity;
        }
      }

      // Mouse Repulsion
      if (mouseRepulsionEnabled) {
        float dx = particles.posX[i] - mousePosition.x;
        float dy = particles.posY[i] - mousePosition.y;
        float distSq = dx * dx + dy * dy;
        float radiusSq = MOUSE_REPULSION_RADIUS * MOUSE_REPULSION_RADIUS;

        if (distSq < radiusSq && distSq > 0.0001f) {
          float dist = std::sqrt(distSq);
          float strength =
              MOUSE_REPULSION_STRENGTH * (1.0f - dist / MOUSE_REPULSION_RADIUS);
          float invDist = 1.0f / dist;
          ax += (dx * invDist) * strength * particles.invMass[i];
          ay += (dy * invDist) * strength * particles.invMass[i];
        }
      }

      // --- Collision Detection (Broad Phase via Spatial Hash) ---
      if (gridEnabled) {
        int cellX = static_cast<int>(particles.posX[i]) >>
                    3; // 8.0f cell size -> shift 3
        int cellY = static_cast<int>(particles.posY[i]) >> 3;

        // Check 3x3 neighbors
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dx = -1; dx <= 1; ++dx) {
            const auto &cell = spatialHash->getCell(cellX + dx, cellY + dy);

            for (uint32_t k = 0; k < cell.count; ++k) {
              uint32_t j = sortedIndices[cell.start + k];

              if (i == j)
                continue;

              float pdx = particles.posX[j] - particles.posX[i];
              float pdy = particles.posY[j] - particles.posY[i];
              float distSq = pdx * pdx + pdy * pdy;

              float radSum = particles.radius[i] + particles.radius[j];
              float radSumSq = radSum * radSum;

              if (distSq < radSumSq && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                float overlap = radSum - dist;
                float invDist = 1.0f / dist;

                float nx = pdx * invDist;
                float ny = pdy * invDist;

                // Spring-like repulsion
                float force = overlap * REPULSION_STRENGTH;

                // Type-specific interactions
                if (type == TYPE_LIQUID && particles.type[j] == TYPE_LIQUID) {
                  // Cohesion (attraction)
                  // Only if slightly separated? Or just reduce repulsion?
                  // Let's add a small attraction force if they are close but
                  // not overlapping too much Actually, standard SPH is complex.
                  // Let's just reduce repulsion for liquids to make them
                  // "squishy"
                  force *= 0.5f;
                } else if (type == TYPE_SAND ||
                           particles.type[j] == TYPE_SAND) {
                  // Friction
                  float friction = 0.1f;
                  particles.velX[i] *= (1.0f - friction);
                  particles.velY[i] *= (1.0f - friction);
                }

                ax -= nx * force * particles.invMass[i];
                ay -= ny * force * particles.invMass[i];
              }
            }
          }
        }
      } else {
        // Brute force fallback (omitted for brevity, assume grid is always on
        // for high perf)
      }

      // Update Velocity
      particles.velX[i] += ax * dt;
      particles.velY[i] += ay * dt;

      // Damping (Air Resistance)
      if (type == TYPE_GAS) {
        particles.velX[i] *= 0.95f;
        particles.velY[i] *= 0.95f;
      }

      // Update Position
      if (type != TYPE_STONE) {
        particles.posX[i] += particles.velX[i] * dt;
        particles.posY[i] += particles.velY[i] * dt;
      }

      // Boundaries
      applyBoundaries(particles, i);
    }
  };

  // Multithreading
  if (multithreadingEnabled) {
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
      numThreads = 4;
    size_t chunkSize = (count + numThreads - 1) / numThreads;

    std::vector<std::future<void>> futures;
    for (unsigned int t = 0; t < numThreads; ++t) {
      size_t start = t * chunkSize;
      size_t end = std::min(start + chunkSize, count);
      if (start < end) {
        futures.push_back(
            std::async(std::launch::async, updateChunk, start, end));
      }
    }

    for (auto &f : futures) {
      f.get();
    }
  } else {
    updateChunk(0, count);
  }
}

void PhysicsEngine::applyBoundaries(ParticleSystem &particles, size_t i) {
  const float width = 1200.0f;
  const float height = 800.0f;
  const float damping = 0.9f;

  if (particles.posX[i] < 0) {
    particles.posX[i] = 0;
    particles.velX[i] *= -damping;
  } else if (particles.posX[i] > width) {
    particles.posX[i] = width;
    particles.velX[i] *= -damping;
  }

  if (particles.posY[i] < 0) {
    particles.posY[i] = 0;
    particles.velY[i] *= -damping;
  } else if (particles.posY[i] > height) {
    particles.posY[i] = height;
    particles.velY[i] *= -damping;
  }
}