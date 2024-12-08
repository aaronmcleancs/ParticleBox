#include "physics.h"
#include <cmath>
#include <vector>
#include <algorithm>

void PhysicsState::updateState(const std::vector<Vec2>& forces, float deltaTime) {
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].velocity.x += (forces[i].x / particles[i].mass) * deltaTime;
        particles[i].velocity.y += (forces[i].y / particles[i].mass) * deltaTime;
        particles[i].position.x += particles[i].velocity.x * deltaTime;
        particles[i].position.y += particles[i].velocity.y * deltaTime;
    }
}

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    const float cellSize = 8.0f;
    
    const int windowWidth = 1200;
    const int windowHeight = 800;
    const int gridWidth = (int)std::ceil(windowWidth / cellSize);
    const int gridHeight = (int)std::ceil(windowHeight / cellSize);

    std::vector<std::vector<int>> cells(gridWidth * gridHeight);

    for (int i = start; i < end; ++i) {
        int cellX = (int)(particles[i].position.x / cellSize);
        int cellY = (int)(particles[i].position.y / cellSize);
        if (cellX < 0) cellX = 0;
        if (cellX >= gridWidth) cellX = gridWidth - 1;
        if (cellY < 0) cellY = 0;
        if (cellY >= gridHeight) cellY = gridHeight - 1;
        cells[cellY * gridWidth + cellX].push_back(i);
    }

    std::vector<Vec2> forces(end - start, Vec2(0, 0));
    for (int i = start; i < end; ++i) {
        Vec2 netForce(0, 0);

        if (gravityEnabled) {
            netForce.y += particles[i].mass * gravity;
        }

        int cellX = (int)(particles[i].position.x / cellSize);
        int cellY = (int)(particles[i].position.y / cellSize);
        if (cellX < 0) cellX = 0;
        if (cellX >= gridWidth) cellX = gridWidth - 1;
        if (cellY < 0) cellY = 0;
        if (cellY >= gridHeight) cellY = gridHeight - 1;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int nx = cellX + dx;
                int ny = cellY + dy;
                if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;

                const std::vector<int>& cellParticles = cells[ny * gridWidth + nx];
                for (int j : cellParticles) {
                    if (j == i) continue;

                    Vec2 direction = particles[j].position - particles[i].position;
                    float distance = direction.magnitude();
                    float combinedRadius = particles[i].radius + particles[j].radius;

                    if (distance < combinedRadius && distance > 0.0f) {
                        Vec2 normal = direction / distance;
                        float overlap = combinedRadius - distance;
                        float totalMass = particles[i].mass + particles[j].mass;
                        float separationScale = overlap / totalMass;
                        particles[i].position -= normal * (separationScale * particles[j].mass);
                        particles[j].position += normal * (separationScale * particles[i].mass);

                        Vec2 relativeVelocity = particles[j].velocity - particles[i].velocity;
                        float restitution = 0.8f;
                        float impulseMagnitude = -(1.0f + restitution) * relativeVelocity.dot(normal) / 
                                                 (1.0f / particles[i].mass + 1.0f / particles[j].mass);
                        Vec2 impulse = normal * impulseMagnitude;

                        particles[i].velocity -= impulse / particles[i].mass;
                        particles[j].velocity += impulse / particles[j].mass;
                    }
                }
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

#ifndef PHYSICS_H
#define PHYSICS_H

#include "particle.h"
#include <vector>

struct PhysicsState {
    std::vector<Particle> particles;
    void updateState(const std::vector<Vec2>& forces, float deltaTime);
};

class PhysicsEngine {
    bool gravityEnabled = true;
public:
    float gravity = 9.81;
    void toggleGravity() { gravityEnabled = !gravityEnabled; }
    std::vector<Vec2> computeForces(std::vector<Particle>& particles, int start, int end);
    void applyBoundaries(Particle& particle);
};

#endif