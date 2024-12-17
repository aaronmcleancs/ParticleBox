#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"
#include <vector>
#include <cmath>
#include <unordered_map> 

struct PhysicsState {
    std::vector<Particle> particles;
    void updateState(const std::vector<Vec2>& forces, float deltaTime);
};

class PhysicsEngine {
    bool gravityEnabled = true;

    
    bool gridEnabled = true;
    bool reducedPairwiseComparisonsEnabled = true;

public:
    float gravity = 9.81;
    void toggleGravity() { gravityEnabled = !gravityEnabled; }
    
    
    void setGridEnabled(bool enabled) { gridEnabled = enabled; }
    void setReducedPairwiseComparisonsEnabled(bool enabled) { reducedPairwiseComparisonsEnabled = enabled; }

    std::vector<Vec2> computeForces(std::vector<Particle>& particles, int start, int end);
    void applyBoundaries(Particle& particle);
};

#endif 