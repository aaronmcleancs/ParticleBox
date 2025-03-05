#include "physics.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdlib>

void PhysicsState::updateState(const std::vector<Vec2>& forces, float deltaTime) {
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].velocity.x += (forces[i].x / particles[i].mass) * deltaTime;
        particles[i].velocity.y += (forces[i].y / particles[i].mass) * deltaTime;
        
        particles[i].position.x += particles[i].velocity.x * deltaTime;
        particles[i].position.y += particles[i].velocity.y * deltaTime;
    }
}

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    std::vector<Vec2> forces(end - start, Vec2(0, 0));

    if (gridEnabled) {
        const float cellSize = 8.0f;
        const int windowWidth = 1200;
        const int windowHeight = 800;
        const int gridWidth = static_cast<int>(std::ceil(windowWidth / cellSize));
        const int gridHeight = static_cast<int>(std::ceil(windowHeight / cellSize));

        std::vector<std::vector<int>> cells(gridWidth * gridHeight);
        for (int i = start; i < end; ++i) {
            // Use bit shifting if positions are positive and cellSize is a power-of-two (8 = 2^3)
            int cellX = static_cast<int>(particles[i].position.x) >> 3;
            int cellY = static_cast<int>(particles[i].position.y) >> 3;
            if (cellX < 0) cellX = 0;
            if (cellX >= gridWidth) cellX = gridWidth - 1;
            if (cellY < 0) cellY = 0;
            if (cellY >= gridHeight) cellY = gridHeight - 1;
            cells[cellY * gridWidth + cellX].push_back(i);
        }

        const float repulsionStrength = 1.5f;

        for (int i = start; i < end; ++i) {
            Vec2 netForce(0, 0);
            if (gravityEnabled) {
                netForce.y += particles[i].mass * gravity;
            }
            int cellX = static_cast<int>(particles[i].position.x) >> 3;
            int cellY = static_cast<int>(particles[i].position.y) >> 3;
            if (cellX < 0) cellX = 0;
            if (cellX >= gridWidth) cellX = gridWidth - 1;
            if (cellY < 0) cellY = 0;
            if (cellY >= gridHeight) cellY = gridHeight - 1;

            int neighborRange = reducedPairwiseComparisonsEnabled ? 1 : 2; 

            for (int dx = -neighborRange; dx <= neighborRange; ++dx) {
                for (int dy = -neighborRange; dy <= neighborRange; ++dy) {
                    int nx = cellX + dx;
                    int ny = cellY + dy;
                    if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;
                    const std::vector<int>& cellParticles = cells[ny * gridWidth + nx];
                    for (int j : cellParticles) {
                        if (j == i) continue;
                        Vec2 direction = particles[j].position - particles[i].position;
                        float distanceSq = direction.x * direction.x + direction.y * direction.y;
                        float combinedRadius = particles[i].radius + particles[j].radius;
                        float combinedRadiusSq = combinedRadius * combinedRadius;
                        if (distanceSq < combinedRadiusSq && distanceSq > 0.0f) {
                            float distance = std::sqrt(distanceSq);
                            Vec2 normal = direction / distance;
                            float overlap = combinedRadius - distance;
                            Vec2 repulsionForce = normal * (repulsionStrength * overlap);
                            netForce -= repulsionForce;
                            if (j >= start && j < end) {
                                forces[j - start] += repulsionForce;
                            }
                        }
                    }
                }
            }
            forces[i - start] = forces[i - start] + netForce;
        }
    } else {
        // Fallback to non-grid based computation remains unchanged
        const float repulsionStrength = 1.5f;
        for (int i = start; i < end; ++i) {
            Vec2 netForce(0, 0);
            if (gravityEnabled) {
                netForce.y += particles[i].mass * gravity;
            }
            for (int j = 0; j < particles.size(); ++j) {
                if (j == i) continue;
                if (reducedPairwiseComparisonsEnabled && (j % 2 != 0)) {
                    continue;
                }
                Vec2 direction = particles[j].position - particles[i].position;
                float distanceSq = direction.x * direction.x + direction.y * direction.y;
                float combinedRadius = particles[i].radius + particles[j].radius;
                float combinedRadiusSq = combinedRadius * combinedRadius;
                if (distanceSq < combinedRadiusSq && distanceSq > 0.0f) {
                    float distance = std::sqrt(distanceSq);
                    Vec2 normal = direction / distance;
                    float overlap = combinedRadius - distance;
                    Vec2 repulsionForce = normal * (repulsionStrength * overlap);
                    netForce -= repulsionForce;
                    if (j >= start && j < end) {
                        forces[j - start] += repulsionForce;
                    }
                }
            }
            forces[i - start] = forces[i - start] + netForce;
        }
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