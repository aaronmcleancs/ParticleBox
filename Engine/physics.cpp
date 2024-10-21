#include "physics.h"
#include <cmath>
#include "physics.h"
#include <cmath>

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    std::vector<Vec2> forces(end - start, Vec2(0, 0));

    for (int i = start; i < end; ++i) {
        Vec2 netForce(0, 0);
        
        if (gravityEnabled) {
            netForce.y += particles[i].mass * gravity;
        }

        for (int j = i + 1; j < particles.size(); ++j) {
            Vec2 direction = particles[j].position - particles[i].position;
            float distance = direction.magnitude();
            float combinedRadius = particles[i].radius + particles[j].radius;
            
            if (distance < combinedRadius) {
                Vec2 normal = direction / distance; 
                float overlap = combinedRadius - distance;
                
                float separationScale = overlap / (particles[i].mass + particles[j].mass);
                particles[i].position -= normal * (separationScale * particles[j].mass);
                particles[j].position += normal * (separationScale * particles[i].mass);
                
                Vec2 relativeVelocity = particles[j].velocity - particles[i].velocity;
                
                float impulseMagnitude = -(1.0f + 0.8f) * relativeVelocity.dot(normal) / 
                                         (1.0f / particles[i].mass + 1.0f / particles[j].mass);
                Vec2 impulse = normal * impulseMagnitude;

                particles[i].velocity -= impulse / particles[i].mass;
                particles[j].velocity += impulse / particles[j].mass;
                particles[i].mass += 0.01f;
            }
        }

        forces[i - start] = netForce;
    }

    return forces;
}

void PhysicsEngine::applyBoundaries(Particle& particle) {
    const int windowWidth = 1200;
    const int windowHeight = 800;
    const float velocityLossFactor = 0.9f;

    if (particle.position.x > windowWidth) {
        particle.position.x = windowWidth; 
        particle.velocity.x *= -velocityLossFactor;
    }

    if (particle.position.x < 0) {
        particle.position.x = 0; 
        particle.velocity.x *= -velocityLossFactor;
    }

    if (particle.position.y < 0) {
        particle.position.y = 0; 
        particle.velocity.y *= -velocityLossFactor;
    }

    if (particle.position.y > windowHeight) {
        particle.position.y = windowHeight; 
        particle.velocity.y *= -velocityLossFactor;
    }
}



