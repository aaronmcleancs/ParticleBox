#include "simulation.h"
#include <SDL.h>
#include <cstdlib> 
#include <cmath>   
#include <ctime>   
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


void Simulation::reset(int count) {
    particles.clear();
    for (int i = 0; i < count; ++i) {
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

    int chunkSize = particles.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? particles.size() : (start + chunkSize);
        futures.push_back(std::async(std::launch::async, updateChunk, start, end));
    }

    for (auto &future : futures) {
        future.get();
    }

    calculateFrameRate();

    /*Frame Rate Regulation - Comment out to cap CPU

    auto currentTime = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime);

    const int targetFrameDuration = 16;

    if (frameDuration.count() < targetFrameDuration) {
        std::this_thread::sleep_for(std::chrono::milliseconds(targetFrameDuration - frameDuration.count()));
    }

    lastFrameTime = std::chrono::steady_clock::now();
    */
    
}

void Simulation::render(SDL_Renderer* renderer) {
    for (size_t i = 0; i < particles.size(); i++) {
        particles[i].render(renderer);
    }
}

Particle Simulation::createRandomParticle() {
    const int windowWidth = 1200;
    const int windowHeight = 800;

    float x = float(rand() % windowWidth);
    float y = float(rand() % windowHeight);
    Vec2 pos(x, y);

    float angle = float(rand() % 360) * M_PI / 180.0f;
    float speed = float(rand() % 50) / 5.0f;
    Vec2 vel(cos(angle) * speed, sin(angle) * speed);

    SDL_Color color = {Uint8(rand() % 256), Uint8(rand() % 256), Uint8(rand() % 256), 255};
    float radius = 2.0f;
    float mass = radius / 5.0f;

    return Particle(pos, vel, color, radius, mass, 0.0, 0.0, 1.0, 0);
}

void Simulation::toggleGravity() {
    physics.toggleGravity();
}

void Simulation::setParticle(int count) {
    if (count < 0 || count > 9999) {
        return;
    }

    int currentCount = particles.size();

    while (currentCount < count) {
        particles.push_back(createRandomParticle());
        currentCount++;
    }

    while (currentCount > count) {
        particles.pop_back();
        currentCount--;
    }
}

void Simulation::calculateFrameRate() {
    auto currentTime = std::chrono::steady_clock::now();
    double secondsPassed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastFrameTime).count();

    if (secondsPassed >= 1.0) {
        frameRate = frameCount / secondsPassed;
        frameCount = 0;
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
