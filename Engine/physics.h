#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"
#include <vector>
#include <cmath>
#include <unordered_map>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

struct PhysicsState {
    std::vector<Particle> particles;
    void updateState(const std::vector<Vec2>& forces, float deltaTime);
};

class PhysicsEngine {
    bool gravityEnabled = true;
    bool gridEnabled = true;
    bool reducedPairwiseComparisonsEnabled = true;
    
    // Constants for spatial partitioning
    static constexpr int CELL_SIZE_SHIFT = 3; // 2^3 = 8
    static constexpr float CELL_SIZE = 8.0f;
    static constexpr int WINDOW_WIDTH = 1200;
    static constexpr int WINDOW_HEIGHT = 800;
    static constexpr float REPULSION_STRENGTH = 1.5f;
    
public:
    float gravity = 9.81;
    void toggleGravity() { gravityEnabled = !gravityEnabled; }
    void setGridEnabled(bool enabled) { gridEnabled = enabled; }
    void setReducedPairwiseComparisonsEnabled(bool enabled) { reducedPairwiseComparisonsEnabled = enabled; }

    // Main physics methods
    std::vector<Vec2> computeForces(std::vector<Particle>& particles, int start, int end);
    void applyBoundaries(Particle& particle);
    
    // Helper methods
    inline int calculateGridWidth() const { 
        return (WINDOW_WIDTH + static_cast<int>(CELL_SIZE) - 1) / static_cast<int>(CELL_SIZE);
    }
    
    inline int calculateGridHeight() const {
        return (WINDOW_HEIGHT + static_cast<int>(CELL_SIZE) - 1) / static_cast<int>(CELL_SIZE);
    }
    
    // Fast cell calculation
    inline int getCellX(const float x) const {
        return static_cast<int>(x) >> CELL_SIZE_SHIFT;
    }
    
    inline int getCellY(const float y) const {
        return static_cast<int>(y) >> CELL_SIZE_SHIFT;
    }
};

#endif 