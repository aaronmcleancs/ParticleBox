#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"
#include <vector>
#include <cmath>    // For advanced mathematical computations

struct PhysicsState {
    std::vector<Particle> particles;
    void updateState(const std::vector<Vec2>& forces, float deltaTime);
};

// Include gravity in the PhysicsEngine
class PhysicsEngine {
     bool gravityEnabled = true;
public:
    float gravity = 9.81;
    void toggleGravity() { gravityEnabled = !gravityEnabled; }
    std::vector<Vec2> computeForces(std::vector<Particle>& particles, int start, int end);
    Vec2 computeInteraction(const Particle& a, const Particle& b);
    void applyBoundaries(Particle& particle);
};


#endif // PHYSICS_H
