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
        
        constexpr float cellSize = 8.0f;
        constexpr int windowWidth = 1200;
        constexpr int windowHeight = 800;
        const int gridWidth = (windowWidth + static_cast<int>(cellSize) - 1) / static_cast<int>(cellSize);
        const int gridHeight = (windowHeight + static_cast<int>(cellSize) - 1) / static_cast<int>(cellSize);
        const int cellShift = 3; 
        
        
        std::vector<int> cellCounts(gridWidth * gridHeight, 0);
        std::vector<int> cellParticles(particles.size());
        std::vector<int> cellStartIndices(gridWidth * gridHeight + 1, 0);

        
        for (int i = start; i < end; ++i) {
            int cellX = static_cast<int>(particles[i].position.x) >> cellShift;
            int cellY = static_cast<int>(particles[i].position.y) >> cellShift;
            cellX = std::max(0, std::min(gridWidth - 1, cellX));
            cellY = std::max(0, std::min(gridHeight - 1, cellY));
            
            ++cellCounts[cellY * gridWidth + cellX];
        }
        
        
        int sum = 0;
        for (int i = 0; i < gridWidth * gridHeight; ++i) {
            cellStartIndices[i] = sum;
            sum += cellCounts[i];
        }
        cellStartIndices[gridWidth * gridHeight] = sum;
        
        
        std::fill(cellCounts.begin(), cellCounts.end(), 0);
        
        
        for (int i = start; i < end; ++i) {
            int cellX = static_cast<int>(particles[i].position.x) >> cellShift;
            int cellY = static_cast<int>(particles[i].position.y) >> cellShift;
            cellX = std::max(0, std::min(gridWidth - 1, cellX));
            cellY = std::max(0, std::min(gridHeight - 1, cellY));
            
            int cellIndex = cellY * gridWidth + cellX;
            int insertIndex = cellStartIndices[cellIndex] + cellCounts[cellIndex]++;
            cellParticles[insertIndex] = i;
        }

        const float repulsionStrength = 1.5f;

#ifdef __APPLE__
        
        simd_float2 gravityForce = {0.0f, 0.0f};
        if (gravityEnabled) {
            gravityForce.y = gravity;
        }
#endif

        int neighborRange = reducedPairwiseComparisonsEnabled ? 1 : 2;

        
        #pragma omp parallel for
        for (int i = start; i < end; ++i) {
#ifdef __APPLE__
            simd_float2 netForce = {0.0f, 0.0f};
            if (gravityEnabled) {
                netForce.y = particles[i].mass * gravity;
            }
            
            // Apply mouse repulsion if enabled
            if (mouseRepulsionEnabled) {
                simd_float2 particlePos = {particles[i].position.x, particles[i].position.y};
                simd_float2 mousePos = {mousePosition.x, mousePosition.y};
                simd_float2 direction = particlePos - mousePos;
                
                float distanceSq = simd_dot(direction, direction);
                float mouseRadiusSq = MOUSE_REPULSION_RADIUS * MOUSE_REPULSION_RADIUS;
                
                if (distanceSq < mouseRadiusSq && distanceSq > 0.0001f) {
                    float invDistance = Vec2::fastInvSqrt(distanceSq);
                    simd_float2 normal = direction * invDistance;
                    float strength = MOUSE_REPULSION_STRENGTH * (1.0f - sqrtf(distanceSq) / MOUSE_REPULSION_RADIUS);
                    simd_float2 repulsionForce = normal * strength;
                    netForce = netForce + repulsionForce;
                }
            }
#else
            Vec2 netForce(0, 0);
            if (gravityEnabled) {
                netForce.y += particles[i].mass * gravity;
            }
            
            // Apply mouse repulsion if enabled
            if (mouseRepulsionEnabled) {
                Vec2 direction = particles[i].position - mousePosition;
                float distanceSq = direction.magnitudeSq();
                float mouseRadiusSq = MOUSE_REPULSION_RADIUS * MOUSE_REPULSION_RADIUS;
                
                if (distanceSq < mouseRadiusSq && distanceSq > 0.0001f) {
                    float invDistance = Vec2::fastInvSqrt(distanceSq);
                    Vec2 normal = direction * invDistance;
                    float strength = MOUSE_REPULSION_STRENGTH * (1.0f - std::sqrt(distanceSq) / MOUSE_REPULSION_RADIUS);
                    Vec2 repulsionForce = normal * strength;
                    netForce += repulsionForce;
                }
            }
#endif

            int cellX = static_cast<int>(particles[i].position.x) >> cellShift;
            int cellY = static_cast<int>(particles[i].position.y) >> cellShift;
            cellX = std::max(0, std::min(gridWidth - 1, cellX));
            cellY = std::max(0, std::min(gridHeight - 1, cellY));

            
            for (int dy = -neighborRange; dy <= neighborRange; ++dy) {
                for (int dx = -neighborRange; dx <= neighborRange; ++dx) {
                    int nx = cellX + dx;
                    int ny = cellY + dy;
                    if (nx < 0 || nx >= gridWidth || ny < 0 || ny >= gridHeight) continue;
                    
                    int cellIndex = ny * gridWidth + nx;
                    int cellStart = cellStartIndices[cellIndex];
                    int cellEnd = cellStartIndices[cellIndex + 1];
                    
                    for (int idx = cellStart; idx < cellEnd; ++idx) {
                        int j = cellParticles[idx];
                        if (j == i) continue;
                        
#ifdef __APPLE__
                        
                        simd_float2 posI = {particles[i].position.x, particles[i].position.y};
                        simd_float2 posJ = {particles[j].position.x, particles[j].position.y};
                        simd_float2 direction = posJ - posI;
                        
                        float distanceSq = simd_dot(direction, direction);
#else
                        Vec2 direction = particles[j].position - particles[i].position;
                        float distanceSq = direction.x * direction.x + direction.y * direction.y;
#endif
                        float combinedRadius = particles[i].radius + particles[j].radius;
                        float combinedRadiusSq = combinedRadius * combinedRadius;
                        
                        if (distanceSq < combinedRadiusSq && distanceSq > 0.0001f) {
                            
                            float invDistance = Vec2::fastInvSqrt(distanceSq);
                            float distance = 1.0f / invDistance;
                            
#ifdef __APPLE__
                            simd_float2 normal = direction * invDistance;
                            float overlap = combinedRadius - distance;
                            simd_float2 repulsionForce = normal * (repulsionStrength * overlap);
                            netForce = netForce - repulsionForce;
#else
                            Vec2 normal = direction * invDistance;
                            float overlap = combinedRadius - distance;
                            Vec2 repulsionForce = normal * (repulsionStrength * overlap);
                            netForce -= repulsionForce;
#endif
                            
                            if (j >= start && j < end) {
                                
                                #pragma omp atomic
                                forces[j - start].x += repulsionForce.x;
                                #pragma omp atomic
                                forces[j - start].y += repulsionForce.y;
                            }
                        }
                    }
                }
            }
            
#ifdef __APPLE__
            forces[i - start].x += netForce.x;
            forces[i - start].y += netForce.y;
#else
            forces[i - start] = forces[i - start] + netForce;
#endif
        }
    } else {
        
        const float repulsionStrength = 1.5f;
        
        
        #pragma omp parallel for
        for (int i = start; i < end; ++i) {
#ifdef __APPLE__
            simd_float2 netForce = {0.0f, 0.0f};
            if (gravityEnabled) {
                netForce.y = particles[i].mass * gravity;
            }
            
            // Apply mouse repulsion if enabled
            if (mouseRepulsionEnabled) {
                simd_float2 particlePos = {particles[i].position.x, particles[i].position.y};
                simd_float2 mousePos = {mousePosition.x, mousePosition.y};
                simd_float2 direction = particlePos - mousePos;
                
                float distanceSq = simd_dot(direction, direction);
                float mouseRadiusSq = MOUSE_REPULSION_RADIUS * MOUSE_REPULSION_RADIUS;
                
                if (distanceSq < mouseRadiusSq && distanceSq > 0.0001f) {
                    float invDistance = Vec2::fastInvSqrt(distanceSq);
                    simd_float2 normal = direction * invDistance;
                    float strength = MOUSE_REPULSION_STRENGTH * (1.0f - sqrtf(distanceSq) / MOUSE_REPULSION_RADIUS);
                    simd_float2 repulsionForce = normal * strength;
                    netForce = netForce + repulsionForce;
                }
            }
#else
            Vec2 netForce(0, 0);
            if (gravityEnabled) {
                netForce.y += particles[i].mass * gravity;
            }
            
            // Apply mouse repulsion if enabled
            if (mouseRepulsionEnabled) {
                Vec2 direction = particles[i].position - mousePosition;
                float distanceSq = direction.magnitudeSq();
                float mouseRadiusSq = MOUSE_REPULSION_RADIUS * MOUSE_REPULSION_RADIUS;
                
                if (distanceSq < mouseRadiusSq && distanceSq > 0.0001f) {
                    float invDistance = Vec2::fastInvSqrt(distanceSq);
                    Vec2 normal = direction * invDistance;
                    float strength = MOUSE_REPULSION_STRENGTH * (1.0f - std::sqrt(distanceSq) / MOUSE_REPULSION_RADIUS);
                    Vec2 repulsionForce = normal * strength;
                    netForce += repulsionForce;
                }
            }
#endif

            for (int j = 0; j < particles.size(); ++j) {
                if (j == i) continue;
                if (reducedPairwiseComparisonsEnabled && (j % 2 != 0)) {
                    continue;
                }
                
#ifdef __APPLE__
                
                simd_float2 posI = {particles[i].position.x, particles[i].position.y};
                simd_float2 posJ = {particles[j].position.x, particles[j].position.y};
                simd_float2 direction = posJ - posI;
                
                float distanceSq = simd_dot(direction, direction);
#else
                Vec2 direction = particles[j].position - particles[i].position;
                float distanceSq = direction.x * direction.x + direction.y * direction.y;
#endif
                
                float combinedRadius = particles[i].radius + particles[j].radius;
                float combinedRadiusSq = combinedRadius * combinedRadius;
                
                if (distanceSq < combinedRadiusSq && distanceSq > 0.0001f) {
                    
                    float invDistance = Vec2::fastInvSqrt(distanceSq);
                    float distance = 1.0f / invDistance;
                    
#ifdef __APPLE__
                    simd_float2 normal = direction * invDistance;
                    float overlap = combinedRadius - distance;
                    simd_float2 repulsionForce = normal * (repulsionStrength * overlap);
                    netForce = netForce - repulsionForce;
#else
                    Vec2 normal = direction * invDistance;
                    float overlap = combinedRadius - distance;
                    Vec2 repulsionForce = normal * (repulsionStrength * overlap);
                    netForce -= repulsionForce;
#endif
                    
                    if (j >= start && j < end) {
                        
                        #pragma omp atomic
                        forces[j - start].x += repulsionForce.x;
                        #pragma omp atomic
                        forces[j - start].y += repulsionForce.y;
                    }
                }
            }
            
#ifdef __APPLE__
            forces[i - start].x += netForce.x;
            forces[i - start].y += netForce.y;
#else
            forces[i - start] = forces[i - start] + netForce;
#endif
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