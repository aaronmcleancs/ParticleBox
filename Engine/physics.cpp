#include "physics.h"
#include <unordered_map>
#include <utility>
#include <cmath>

namespace std {
    template<>
    struct hash<std::pair<int, int>> {
        std::size_t operator()(const std::pair<int, int>& pair) const noexcept {
            auto hash1 = std::hash<int>{}(pair.first);
            auto hash2 = std::hash<int>{}(pair.second);
            return hash1 ^ (hash2 << 1);
        }
    };
}

std::vector<Vec2> PhysicsEngine::computeForces(std::vector<Particle>& particles, int start, int end) {
    const float cellSize = 50.0f;
    std::unordered_map<std::pair<int, int>, std::vector<int>> cellMap;

    for (int i = start; i < end; ++i) {
        int cellX = particles[i].position.x / cellSize;
        int cellY = particles[i].position.y / cellSize;
        cellMap[{cellX, cellY}].push_back(i);
    }

    std::vector<Vec2> forces(end - start, Vec2(0, 0));
    for (int i = start; i < end; ++i) {
        Vec2 netForce(0, 0);

        if (gravityEnabled) {
            netForce.y += particles[i].mass * gravity;
        }

        int cellX = particles[i].position.x / cellSize;
        int cellY = particles[i].position.y / cellSize;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                auto it = cellMap.find({cellX + dx, cellY + dy});
                if (it == cellMap.end()) continue;

                for (int j : it->second) {
                    if (i == j) continue;

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



