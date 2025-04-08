#include "simulation.h"
#include <SDL.h>
#include <cstdlib> 
#include <cmath>   
#include <ctime>   
#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <random>
#include <algorithm>
#include <chrono>

Simulation::Simulation() 
    : running(false), 
      frameCount(0), 
      frameRate(0.0f),
      multithreadingEnabled(true), 
      gridEnabled(true),
      reducedPairwiseComparisonsEnabled(true) 
{
    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    lastFrameTime = std::chrono::steady_clock::now();
    simulation_speed = 0.0016f;
    physics.setGridEnabled(gridEnabled);
    physics.setReducedPairwiseComparisonsEnabled(reducedPairwiseComparisonsEnabled);
}

void Simulation::start() {
    running = true;
}

void Simulation::stop() {
    running = false;
}

void Simulation::reset(int count) {
    particles.clear();
    particles.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        particles.push_back(createRandomParticle());
    }
}

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#endif

void Simulation::update(double deltaTime) {
    if (!running) return;

    const size_t totalParticles = particles.size();
    if (totalParticles == 0) {
        calculateFrameRate();
        return;
    }

    if (multithreadingEnabled) {
#ifdef __APPLE__
        
        const size_t numThreads = std::thread::hardware_concurrency();
        const size_t effectiveThreads = std::min(numThreads, static_cast<size_t>(8)); 
        const size_t chunkSize = (totalParticles + effectiveThreads - 1) / effectiveThreads;
        
        
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
        
        
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
        
        size_t processed = 0;
        for (size_t i = 0; i < effectiveThreads; ++i) {
            const size_t start = processed;
            const size_t end = std::min(start + chunkSize, totalParticles);
            processed = end;
            
            if (start >= end) break;
            
            dispatch_async(queue, ^{
                
                std::vector<Vec2> forces = physics.computeForces(particles, static_cast<int>(start), static_cast<int>(end));
                
                
                for (size_t j = start; j < end; ++j) {
                    Particle &p = particles[j];
                    
                    simd_float2 vel = {p.velocity.x, p.velocity.y};
                    simd_float2 force = {forces[j - start].x, forces[j - start].y};
                    simd_float2 deltaVel = force * (p.invMass * static_cast<float>(deltaTime));
                    vel = vel + deltaVel;
                    
                    
                    simd_float2 pos = {p.position.x, p.position.y};
                    simd_float2 deltaPos = vel * static_cast<float>(deltaTime);
                    pos = pos + deltaPos;
                    
                    
                    p.velocity.x = vel.x;
                    p.velocity.y = vel.y;
                    p.position.x = pos.x;
                    p.position.y = pos.y;
                    
                    physics.applyBoundaries(p);
                }
                
                
                dispatch_semaphore_signal(semaphore);
            });
        }
        
        
        for (size_t i = 0; i < processed / chunkSize; ++i) {
            dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        }
#else
        
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) {
            numThreads = 4;
        }
        
        const size_t chunkSize = (totalParticles + numThreads - 1) / numThreads;
        std::vector<std::future<void>> futures;
        futures.reserve(numThreads);

        auto updateChunk = [this, deltaTime](size_t start, size_t end) {
            if (end <= start) return;
            std::vector<Vec2> forces = physics.computeForces(particles, static_cast<int>(start), static_cast<int>(end));
            
            #pragma omp simd
            for (size_t i = start; i < end; ++i) {
                Particle &p = particles[i];
                p.velocity.x += (forces[i - start].x * p.invMass) * (float)deltaTime;
                p.velocity.y += (forces[i - start].y * p.invMass) * (float)deltaTime;
                p.position.x += p.velocity.x * (float)deltaTime;
                p.position.y += p.velocity.y * (float)deltaTime;
                
                physics.applyBoundaries(p);
            }
        };

        size_t processed = 0;
        for (unsigned int i = 0; i < numThreads; ++i) {
            const size_t start = processed;
            const size_t end = std::min(start + chunkSize, totalParticles);
            processed = end;
            if (start < end) {
                futures.push_back(std::async(std::launch::async, updateChunk, start, end));
            } else {
                break;
            }
        }

        for (auto& future : futures) {
            future.get();
        }
#endif
    } else {
        
        std::vector<Vec2> forces = physics.computeForces(particles, 0, static_cast<int>(totalParticles));
        
#ifdef __APPLE__
        for (size_t i = 0; i < totalParticles; ++i) {
            Particle &p = particles[i];
            
            
            simd_float2 vel = {p.velocity.x, p.velocity.y};
            simd_float2 force = {forces[i].x, forces[i].y};
            simd_float2 deltaVel = force * (p.invMass * static_cast<float>(deltaTime));
            vel = vel + deltaVel;
            
            
            simd_float2 pos = {p.position.x, p.position.y};
            simd_float2 deltaPos = vel * static_cast<float>(deltaTime);
            pos = pos + deltaPos;
            
            
            p.velocity.x = vel.x;
            p.velocity.y = vel.y;
            p.position.x = pos.x;
            p.position.y = pos.y;
            
            physics.applyBoundaries(p);
        }
#else
        #pragma omp simd
        for (size_t i = 0; i < totalParticles; ++i) {
            Particle &p = particles[i];
            p.velocity.x += (forces[i].x * p.invMass) * (float)deltaTime;
            p.velocity.y += (forces[i].y * p.invMass) * (float)deltaTime;
            p.position.x += p.velocity.x * (float)deltaTime;
            p.position.y += p.velocity.y * (float)deltaTime;
            
            physics.applyBoundaries(p);
        }
#endif
    }

    calculateFrameRate();
    
    
    auto currentTime = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - lastFrameTime
    );
    
    
    constexpr int targetFrameDuration = 16666; 
    if (frameDuration.count() < targetFrameDuration) {
#ifdef __APPLE__
        
        mach_timebase_info_data_t timebase;
        mach_timebase_info(&timebase);
        uint64_t delay_ns = (targetFrameDuration - frameDuration.count()) * 1000;
        uint64_t deadline = mach_absolute_time() + delay_ns * timebase.denom / timebase.numer;
        dispatch_time_t dispatchDeadline = dispatch_time(DISPATCH_TIME_NOW, delay_ns);
        dispatch_after(dispatchDeadline, dispatch_get_main_queue(), ^{});
#else
        std::this_thread::sleep_for(
            std::chrono::microseconds(targetFrameDuration - frameDuration.count())
        );
#endif
    }
    
    lastFrameTime = std::chrono::steady_clock::now();
}

void Simulation::render(SDL_Renderer* renderer) {
    for (Particle &p : particles) {
        p.render(renderer);
    }
}

Vec2 Simulation::getAverageVelocity() const {
    size_t n = particles.size();
    if (n == 0) {
        return Vec2(0.0f, 0.0f);
    }

    Vec2 sumVelocity(0.0f, 0.0f);
    for (const Particle &p : particles) {
        sumVelocity.x += p.velocity.x;
        sumVelocity.y += p.velocity.y;
    }

    sumVelocity.x /= static_cast<float>(n);
    sumVelocity.y /= static_cast<float>(n);

    return sumVelocity;
}

Particle Simulation::createRandomParticle() {
    static thread_local std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)) ^ 
                                         static_cast<unsigned>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    static std::uniform_real_distribution<float> distX(0.0f, 1200.0f);
    static std::uniform_real_distribution<float> distY(0.0f, 800.0f);
    static std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    static std::uniform_real_distribution<float> distSpeed(0.0f, 50.0f);
    static std::uniform_int_distribution<int> distColor(0, 255);

    float x = distX(rng);
    float y = distY(rng);
    Vec2 pos(x, y);

    float angle = distAngle(rng);
    float speed = distSpeed(rng);
    Vec2 vel(std::cos(angle) * speed, std::sin(angle) * speed);

    SDL_Color color = {
        static_cast<Uint8>(distColor(rng)),
        static_cast<Uint8>(distColor(rng)),
        static_cast<Uint8>(distColor(rng)),
        255
    };
    float radius = 2.0f;
    float mass = radius / 5.0f;

    return Particle(pos, vel, color, radius, mass, 0.0f, 0.0f, 1.0f, 0);
}

void Simulation::toggleGravity() {
    physics.toggleGravity();
}

void Simulation::setParticle(int count) {
    if (count < 0 || count > 9999) {
        return;
    }
    int currentCount = (int)particles.size();
    if (count > currentCount) {
        particles.reserve((size_t)count);
        for (int i = currentCount; i < count; ++i) {
            particles.push_back(createRandomParticle());
        }
    } else {
        particles.erase(particles.begin() + count, particles.end());
    }
}

void Simulation::calculateFrameRate() {
    static std::chrono::steady_clock::time_point frameRateStartTime = std::chrono::steady_clock::now();
    frameCount++;

    auto currentTime = std::chrono::steady_clock::now();
    double secondsPassed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - frameRateStartTime).count();

    if (secondsPassed >= 1.0) {
        frameRate = frameCount / (float)secondsPassed;
        frameCount = 0;
        frameRateStartTime = currentTime;
    }
}

void Simulation::spawnParticlesAtMouse(int x, int y, int count) {
    particles.reserve(particles.size() + (size_t)count);
    for (int i = 0; i < count; ++i) {
        particles.push_back(createParticleAtPosition(x, y));
    }
}

Particle Simulation::createParticleAtPosition(int x, int y) {
    static thread_local std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)) ^
                                         static_cast<unsigned>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    static std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * (float)M_PI);
    static std::uniform_real_distribution<float> distSpeed(0.0f, 10.0f);
    static std::uniform_int_distribution<int> distColor(0, 255);

    Vec2 pos(static_cast<float>(x), static_cast<float>(y));
    float angle = distAngle(rng);
    float speed = distSpeed(rng);

    Vec2 vel(std::cos(angle) * speed, std::sin(angle) * speed);

    SDL_Color color = {
        static_cast<Uint8>(distColor(rng)),
        static_cast<Uint8>(distColor(rng)),
        static_cast<Uint8>(distColor(rng)),
        255
    };

    float radius = 2.0f;
    float mass = radius / 2.0f; 
    float dipoleMoment = 0.0f;
    float exclusionConstant = 0.0f;
    float repulsionFactor = 1.0f;
    int type = 0;

    return Particle(pos, vel, color, radius, mass, dipoleMoment, exclusionConstant, repulsionFactor, type);
}

float Simulation::getFrameRate() const {
    return frameRate;
}

int Simulation::getParticleCount() const {
    return (int)particles.size();
}