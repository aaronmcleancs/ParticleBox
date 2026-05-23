#ifndef SIMULATION_H
#define SIMULATION_H

#include "input_state.h"
#include "particle.h"
#include "physics.h"
#include "vec2.h"

#include <chrono>
#include <random>

class Simulation {
public:
  Simulation();

  void start();
  void stop();
  bool isRunning() const { return running_; }

  // Refill the world with this many randomised particles.
  void reset(int particleCount);

  // Clear every particle without resetting any other state.
  void clearParticles();

  // Set the velocity of all particles to zero.
  void freezeAll();

  void update(float frameDt);
  void render(struct SDL_Renderer *renderer);

  // Spawn a few particles centred on (x, y) with a brush-style scatter.
  void spawnBrush(int x, int y, int count, float brushRadius, ParticleType t);

  // Erase any particles inside the brush at (x, y).
  void eraseBrush(int x, int y, float brushRadius);

  // Single-particle convenience used for menus.
  void spawnAt(float x, float y, ParticleType t);

  // Trigger an explosion at (x, y) on the next physics step.
  void triggerExplosion(float x, float y);

  // Toggles forwarded to the physics engine.
  void toggleGravity();
  void toggleMultithreading();
  void toggleGrid();

  // Read-only access to shared input state (the input manager mutates this).
  InputState       &input()       { return input_; }
  const InputState &input() const { return input_; }

  // Diagnostics
  float getFrameRate() const     { return frameRate_; }
  float getAvgUpdateMs() const   { return avgUpdateMs_; }
  int   getParticleCount() const { return static_cast<int>(particles_.count); }
  Vec2  getAverageVelocity() const;

  bool isMultithreadingEnabled() const { return input_.multithreadEnabled; }
  bool isGridEnabled() const           { return input_.gridEnabled; }

  // Direct (read-only) access for the renderer.
  const ParticleSystem &particles() const { return particles_; }

private:
  void createRandomParticle();

  ParticleSystem particles_;
  PhysicsEngine  physics_;
  InputState     input_;

  bool  running_ = true;
  float frameRate_   = 0.0f;
  float avgUpdateMs_ = 0.0f;
  int   frameCount_  = 0;
  std::chrono::steady_clock::time_point fpsStart_;

  std::mt19937 rng_;
};

#endif
