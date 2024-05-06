#include "physics.h"
#include <cmath>

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    std::vector<Vec2> forces(end - start, Vec2(0, 0)); // Initialize forces for the subset

    for (int i = start; i < end; ++i) {
        Vec2 netForce(0, 0); // Start with no force
        for (int j = 0; j < particles.size(); ++j) {
            if (i != j) {
                Vec2 force = computeInteraction(particles[i], particles[j]);
                netForce += force; // Sum up all interaction forces
            }
        }
        // Apply gravity if enabled
        if (gravityEnabled) {
            netForce.y += particles[i].mass * gravity; // Force due to gravity, assuming gravity acts downward
        }
        forces[i - start] = netForce; // Assign computed net force to this particle
    }
    return forces;
}

Vec2 PhysicsEngine::computeInteraction(const Particle& a, const Particle& b) {
    Vec2 delta = a.position - b.position;
    float dist = delta.magnitude();
    float forceMagnitude = -a.mass * b.mass / (dist * dist); // Gravitational-like attraction
    return delta.norm() * forceMagnitude;
}

void PhysicsEngine::applyBoundaries(Particle& particle) {
    const int windowWidth = 1200;
    const int windowHeight = 800;

    // Right boundary
    if (particle.position.x > windowWidth) {
        particle.position.x = windowWidth; // Reposition to avoid sticking to the edge
        particle.velocity.x *= -1; // Reverse the velocity
    }

    // Left boundary
    if (particle.position.x < 0) {
        particle.position.x = 0; // Reposition to avoid sticking to the edge
        particle.velocity.x *= -1; // Reverse the velocity
    }

    // Top boundary
    if (particle.position.y < 0) {
        particle.position.y = 0; // Reposition to avoid sticking to the edge
        particle.velocity.y *= -1; // Reverse the velocity
    }

    // Bottom boundary
    if (particle.position.y > windowHeight) {
        particle.position.y = windowHeight; // Reposition to avoid sticking to the edge
        if (particle.velocity.y > 0) { // Check if velocity is downward; reverse if so
            particle.velocity.y *= -1; // Reverse the velocity
        }
    }
}



