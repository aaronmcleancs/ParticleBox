#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"
#include "spatial_hash.h"
#include <memory>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

class PhysicsEngine {
  bool gravityEnabled = true;
  bool gridEnabled = true; // Now controls SpatialHash usage
  bool multithreadingEnabled = true;
  bool reducedPairwiseEnabled =
      true; // If true, use Spatial Hash. If false, use Brute Force (if grid is
            // also off? or just independent?) Actually, let's make gridEnabled
            // control SpatialHash build, and this flag control whether we use
            // it? Simpler: GridEnabled = Spatial Hash. ReducedPairwise =
            // Optimization within cell? Let's just add the flags for now.

  // Constants
  static constexpr float REPULSION_STRENGTH = 1.5f;
  static constexpr float MOUSE_REPULSION_STRENGTH = 100.0f;
  static constexpr float MOUSE_REPULSION_RADIUS = 100.0f;

  Vec2 mousePosition{0.0f, 0.0f};
  bool mouseRepulsionEnabled = false;

  // Spatial Hash
  std::unique_ptr<SpatialHash> spatialHash;
  std::vector<uint32_t> sortedIndices; // For spatial hash

public:
  float gravity = 9.81f;

  PhysicsEngine();

  void toggleGravity() { gravityEnabled = !gravityEnabled; }
  void setGridEnabled(bool enabled) { gridEnabled = enabled; }
  void setMultithreadingEnabled(bool enabled) {
    multithreadingEnabled = enabled;
  }
  void setReducedPairwiseEnabled(bool enabled) {
    reducedPairwiseEnabled = enabled;
  }

  void setMousePosition(float x, float y) {
    mousePosition.x = x;
    mousePosition.y = y;
    mouseRepulsionEnabled = true;
  }
  void disableMouseRepulsion() { mouseRepulsionEnabled = false; }

  // Main update function
  void update(ParticleSystem &particles, float deltaTime);

  // Helper for boundary conditions
  void applyBoundaries(ParticleSystem &particles, size_t index);
};

#endif