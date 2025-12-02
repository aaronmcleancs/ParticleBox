#ifndef SIMULATION_H
#define SIMULATION_H

#include "particle.h"
#include "physics.h"
#include <chrono>
#include <vector>

class Simulation {
public:
  Simulation();

  void start();
  void stop();
  void reset(int particleCount);
  void update(double deltaTime);
  void render(SDL_Renderer *renderer);

  // Interaction
  void spawnParticlesAtMouse(int x, int y, int count);
  void updateMousePosition(int x, int y);
  void disableMouseRepulsion();

  // Toggles
  void toggleGravity();
  void toggleMultithreading(); // Kept for interface compatibility, though
                               // physics engine handles threading now
  void toggleGrid();
  void toggleReducedPairwiseComparisons(); // Kept for interface compatibility

  // Getters
  float getFrameRate() const;
  int getParticleCount() const;
  Vec2 getAverageVelocity() const;

  // Setters
  void setParticle(int count);

  // State
  bool isMultithreadingEnabled() const { return multithreadingEnabled; }
  bool isGridEnabled() const { return gridEnabled; }
  bool isReducedPairwiseComparisonsEnabled() const {
    return reducedPairwiseComparisonsEnabled;
  }

private:
  bool running;
  bool multithreadingEnabled;
  bool gridEnabled;
  bool reducedPairwiseComparisonsEnabled;

  PhysicsEngine physics;
  ParticleSystem particles;

  // Time keeping
  std::chrono::steady_clock::time_point lastFrameTime;
  int frameCount;
  float frameRate;
  void calculateFrameRate();

  // Helpers
  void createRandomParticle();
  void createParticleAtPosition(int x, int y);
};

#endif