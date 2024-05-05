#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include "particle.h"
#include "physics.h"

class Simulation {
    PhysicsEngine physics;
    std::vector<Particle> particles;
    bool running;
    std::chrono::steady_clock::time_point lastFrameTime;
    int frameCount;
    float frameRate;

public:
    Simulation();
    void start();
    void stop();
    void update(double deltaTime);
    void calculateFrameRate();
    float getFrameRate() const;
    void render(SDL_Renderer* renderer);
    void reset();
    Particle createRandomParticle();
    int getParticleCount() const;
    void toggleGravity();
    void addParticle();
    void removeParticle();
};

#endif
