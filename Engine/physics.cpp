#include "physics.h"
#include <cmath>  // For pow function

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles) {
    std::vector<Vec2> forces(particles.size(), Vec2(0, gravityEnabled ? gravity : 0));  // Conditional gravity application

    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            Vec2 force = computeInteraction(particles[i], particles[j]);
            Vec2 dipoleForce = computeDipoleInteraction(particles[i], particles[j]);
            Vec2 exclusionForce = computeExclusionForce(particles[i], particles[j]);

            forces[i] += force + dipoleForce + exclusionForce;
            forces[j] -= force + dipoleForce + exclusionForce;
        }
        applyBoundaries(particles[i]);
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
    // Define the right boundary of the control panel
    const int controlPanelWidth = 200;
    const int rightBoundary = controlPanelWidth;

    // Left boundary of the simulation area
    if (particle.position.x < rightBoundary) {
        particle.velocity.x *= -1;  // Reverse direction if particle tries to enter control area
        particle.position.x = rightBoundary; // Reposition particle at the boundary to avoid sticking to the edge
    }

    // Right boundary of the window
    const int windowWidth = 1200; // Total window width including control panel
    if (particle.position.x > windowWidth) {
        particle.velocity.x *= -1;  // Reverse direction if particle hits the right edge
        particle.position.x = windowWidth; // Reposition to avoid sticking to the edge
    }

    // Top boundary of the window
    if (particle.position.y < 0) {
        particle.velocity.y *= -1;  // Reverse direction if particle hits the top edge
        particle.position.y = 0;  // Reposition to avoid sticking to the edge
    }

    // Bottom boundary of the window
    const int windowHeight = 600; // Total height of the window
    if (particle.position.y > windowHeight) {
        particle.velocity.y *= -1;  // Reverse direction if particle hits the bottom edge
        particle.position.y = windowHeight; // Reposition to avoid sticking to the edge
    }
}
