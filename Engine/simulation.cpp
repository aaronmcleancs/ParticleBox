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

void Simulation::update(double deltaTime) {
    if (!running) return;

    const int numThreads = 8;
    
    const size_t totalParticles = particles.size();
    if (totalParticles == 0) {
        calculateFrameRate();
        return;
    }

    if (multithreadingEnabled) {
        const size_t chunkSize = (totalParticles + numThreads - 1) / numThreads;
        std::vector<std::future<void>> futures;
        futures.reserve(numThreads);

        auto updateChunk = [this, deltaTime](size_t start, size_t end) {
            if (end <= start) return;
            std::vector<Vec2> forces = physics.computeForces(particles, static_cast<int>(start), static_cast<int>(end));
            for (size_t i = start; i < end; ++i) {
                Particle &p = particles[i];
                p.velocity.x += (forces[i - start].x / p.mass) * (float)deltaTime;
                p.velocity.y += (forces[i - start].y / p.mass) * (float)deltaTime;
                p.position.x += p.velocity.x * (float)deltaTime;
                p.position.y += p.velocity.y * (float)deltaTime;

                physics.applyBoundaries(p);
            }
        };

        size_t processed = 0;
        for (int i = 0; i < numThreads; ++i) {
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
    } else {
        
        std::vector<Vec2> forces = physics.computeForces(particles, 0, static_cast<int>(totalParticles));
        for (size_t i = 0; i < totalParticles; ++i) {
            Particle &p = particles[i];
            p.velocity.x += (forces[i].x / p.mass) * (float)deltaTime;
            p.velocity.y += (forces[i].y / p.mass) * (float)deltaTime;
            p.position.x += p.velocity.x * (float)deltaTime;
            p.position.y += p.velocity.y * (float)deltaTime;

            physics.applyBoundaries(p);
        }
    }

    calculateFrameRate();

    auto currentTime = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - lastFrameTime
    );

    constexpr int targetFrameDuration = 16;
    if (frameDuration.count() < targetFrameDuration) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(targetFrameDuration - frameDuration.count())
        );
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
