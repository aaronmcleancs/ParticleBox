#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include "particle.h"
#include "physics.h"

class Simulation {
    PhysicsEngine physics;
    std::vector<Particle> particles;
    bool running;  // State to manage the running state of the simulation

public:
    Simulation();
    void start();
    void stop();
    void update(double deltaTime);
    void render(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event);
    void reset();
    Particle createRandomParticle();
    int getParticleCount() const;
    float getFrameRate() const;
};

#endif
