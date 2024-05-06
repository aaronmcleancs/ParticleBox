#include "simulation.h"
#include <SDL.h>
#include <cstdlib>  // For rand()
#include <cmath>    // For cos, sin, M_PI
#include <ctime>    // For time()
#include <vector>
#include <thread>
#include <future>
#include <numeric>

Simulation::Simulation() : running(false), frameCount(0), frameRate(0.0f) {
    srand(time(nullptr));
    lastFrameTime = std::chrono::steady_clock::now();
    simulation_speed = 0.0016;
}

void Simulation::start() {
    running = true;
}

void Simulation::stop() {
    running = false;
}

void Simulation::reset() {
    particles.clear();
    for (int i = 0; i < 100; ++i) {
        particles.push_back(createRandomParticle());
    }
}

void Simulation::update(double deltaTime) {
    if (!running) return;

    int numThreads = 24;
    std::vector<std::future<void>> futures;

    auto updateChunk = [this, deltaTime](int start, int end) {
        std::vector<Vec2> forces = physics.computeForces(particles, start, end);
        for (int i = start; i < end; ++i) {
            particles[i].update(forces[i-start], deltaTime);
            physics.applyBoundaries(particles[i]);
        }
    };

    // Calculate the appropriate chunk size
    int chunkSize = particles.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? particles.size() : (start + chunkSize);
        futures.push_back(std::async(std::launch::async, updateChunk, start, end));
    }

    // Wait for all threads to finish
    for (auto &future : futures) {
        future.get();
    }

    calculateFrameRate();
}


void Simulation::render(SDL_Renderer* renderer) {
    for (size_t i = 0; i < particles.size(); i++) {
        particles[i].render(renderer);
    }
}

Particle Simulation::createRandomParticle() {
    const int windowWidth = 1200; // Window width remains the same
    const int windowHeight = 800; // Window height remains the same

    float x = float(rand() % windowWidth);
    float y = float(rand() % windowHeight);
    Vec2 pos(x, y);

    // Angle and speed calculations remain unchanged
    float angle = float(rand() % 360) * M_PI / 180.0f;
    float speed = float(rand() % 50) / 10.0f;
    Vec2 vel(cos(angle) * speed, sin(angle) * speed);

    // Color generation remains unchanged
    SDL_Color color = {Uint8(rand() % 256), Uint8(rand() % 256), Uint8(rand() % 256), 255};
    float radius = 3.0f;
    float mass = radius / 5.0f;

    return Particle(pos, vel, color, radius, mass, 0.0, 0.0, 1.0, 0);
}

void Simulation::toggleGravity() {
    physics.toggleGravity();
}

void Simulation::addParticle() {
    if (particles.size() < 1000) {  // Ensure there's an upper limit for particles
        particles.push_back(createRandomParticle());
    }
}

void Simulation::removeParticle() {
    if (!particles.empty()) {
        particles.pop_back();
    }
}

void Simulation::calculateFrameRate() {
    auto currentTime = std::chrono::steady_clock::now();
    double secondsPassed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastFrameTime).count();

    if (secondsPassed >= 1.0) {
        frameRate = frameCount / secondsPassed;
        frameCount = 0;
        lastFrameTime = currentTime;
    } else {
        frameCount++;
    }
}

float Simulation::getFrameRate() const {
    return frameRate;
}

int Simulation::getParticleCount() const {
    return particles.size();
}
