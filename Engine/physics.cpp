#include "physics.h"
#include <cmath>

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    std::vector<Vec2> forces(end - start, Vec2(0, 0));

    for (int i = start; i < end; ++i) {
        Vec2 netForce(0, 0);
        for (int j = 0; j < particles.size(); ++j) {
            if (i != j) {
                Vec2 interactionForce = computeInteraction(particles[i], particles[j]);
                Vec2 repulsionForce = computeRepulsion(particles[i], particles[j]);
                netForce += interactionForce;
                netForce += repulsionForce;  
            }
        }
        if (gravityEnabled) {
            netForce.y += particles[i].mass * gravity;
        }
        forces[i - start] = netForce;
    }
    return forces;
}

Vec2 PhysicsEngine::computeRepulsion(const Particle& a, const Particle& b) {
    Vec2 delta = a.position - b.position;
    float dist = delta.magnitude();
    if (dist < 1e-6) return Vec2(0, 0); 

    float repulsionCoefficient = 50;
    float attractionCoefficient = 50;  
    float attractionRange = 50;  
    float repulsionRange = 10;  

    float repulsionForce = (dist < repulsionRange) ? (repulsionCoefficient / (dist * dist * dist)) : 0;

    float attractionForce = (dist > repulsionRange && dist < attractionRange) ? (-attractionCoefficient / (dist * dist)) : 0;

    float netForceMagnitude = repulsionForce + attractionForce;

    return delta.norm() * netForceMagnitude;
}


Vec2 PhysicsEngine::computeInteraction(const Particle& a, const Particle& b) {
    Vec2 delta = a.position - b.position;
    float dist = delta.magnitude();
    float forceMagnitude = -a.mass * b.mass / (dist * dist);
    return delta.norm() * forceMagnitude;
}

void PhysicsEngine::applyBoundaries(Particle& particle) {
    const int windowWidth = 1200;
    const int windowHeight = 800;

    if (particle.position.x > windowWidth) {
        particle.position.x = windowWidth; 
        particle.velocity.x *= -1; 
    }

    if (particle.position.x < 0) {
        particle.position.x = 0; 
        particle.velocity.x *= -1; 
    }

    if (particle.position.y < 0) {
        particle.position.y = 0; 
        particle.velocity.y *= -1; 
    }


    if (particle.position.y > windowHeight) {
        particle.position.y = windowHeight; 
        if (particle.velocity.y > 0) { 
            particle.velocity.y *= -1; 
        }
    }
}



