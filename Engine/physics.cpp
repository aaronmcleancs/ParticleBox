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

Vec2 PhysicsEngine::computeDipoleInteraction(const Particle& a, const Particle& b) {
    Vec2 delta = b.position - a.position;
    float distance = delta.magnitude();
    if (distance == 0) return Vec2(); // Prevent division by zero

    // Normalize the distance vector
    Vec2 r_hat = delta.norm();

    // Retrieve the dipole moments from the particles
    Vec2 dipoleA = a.dipoleMoment;
    Vec2 dipoleB = b.dipoleMoment;

    // Calculate components of the dipole interaction
    float scalarProduct = dipoleA.dot(dipoleB);
    float dipoleA_projected = dipoleA.dot(r_hat);
    float dipoleB_projected = dipoleB.dot(r_hat);

    // Calculate the magnitude of the force using the detailed dipole-dipole interaction formula
    float forceMagnitude = (3 * dipoleA_projected * dipoleB_projected - scalarProduct) / pow(distance, 3);

    // Direction of the force is along the line connecting the particles
    return r_hat * forceMagnitude;
}


Vec2 PhysicsEngine::computeExclusionForce(const Particle& a, const Particle& b) {
    Vec2 delta = b.position - a.position;
    float distance = delta.magnitude();
    if (distance == 0) return Vec2(); // Avoid division by zero and self-interaction

    // Example using the Lennard-Jones potential for repulsion only
    float epsilon = 1.0f; // Adjust this parameter to scale the force intensity
    float sigma = (a.radius + b.radius) / 2; // Effective diameter of interaction

    if (distance < 2 * sigma) { // Consider force calculation within a reasonable cutoff
        float forceMagnitude = 4 * epsilon * pow(sigma / distance, 12);
        return delta.norm() * forceMagnitude;
    }

    return Vec2(); // No force if particles are not close enough
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



