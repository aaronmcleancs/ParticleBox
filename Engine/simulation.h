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

    
    bool multithreadingEnabled;
    bool gridEnabled;
    bool reducedPairwiseComparisonsEnabled;

public:
    Simulation();
    void start();
    void stop();
    void update(double deltaTime);
    void calculateFrameRate();
    float getFrameRate() const;
    void render(SDL_Renderer* renderer);
    void reset(int count);
    void spawnParticlesAtMouse(int x, int y, int count);
    Particle createParticleAtPosition(int x, int y);
    Particle createRandomParticle();
    int getParticleCount() const;
    void toggleGravity();
    void setParticle(int count);
    float simulation_speed;
    Vec2 getAverageVelocity() const;
    
    // Mouse position handling for repulsion
    void updateMousePosition(int x, int y) { physics.setMousePosition(static_cast<float>(x), static_cast<float>(y)); }
    void disableMouseRepulsion() { physics.disableMouseRepulsion(); }

    
    void toggleMultithreading() { multithreadingEnabled = !multithreadingEnabled; }
    void toggleGrid() { gridEnabled = !gridEnabled; physics.setGridEnabled(gridEnabled); }
    void toggleReducedPairwiseComparisons() { reducedPairwiseComparisonsEnabled = !reducedPairwiseComparisonsEnabled; physics.setReducedPairwiseComparisonsEnabled(reducedPairwiseComparisonsEnabled); }

    
    bool isMultithreadingEnabled() const { return multithreadingEnabled; }
    bool isGridEnabled() const { return gridEnabled; }
    bool isReducedPairwiseComparisonsEnabled() const { return reducedPairwiseComparisonsEnabled; }
};

#endif