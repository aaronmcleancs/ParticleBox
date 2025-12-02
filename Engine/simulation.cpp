#include "simulation.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>

Simulation::Simulation()
    : running(false), multithreadingEnabled(true), gridEnabled(true),
      reducedPairwiseComparisonsEnabled(true), frameCount(0), frameRate(0.0f),
      particles(10000) // Initial capacity
{
  lastFrameTime = std::chrono::steady_clock::now();
  physics.setGridEnabled(gridEnabled);
}

void Simulation::start() { running = true; }

void Simulation::stop() { running = false; }

void Simulation::reset(int count) {
  particles.clear();
  particles.reserve(count);
  for (int i = 0; i < count; ++i) {
    createRandomParticle();
  }
}

void Simulation::update(double deltaTime) {
  if (!running)
    return;

  physics.update(particles, static_cast<float>(deltaTime));
  calculateFrameRate();
}

void Simulation::render(SDL_Renderer *renderer) { particles.render(renderer); }

void Simulation::spawnParticlesAtMouse(int x, int y, int count) {
  for (int i = 0; i < count; ++i) {
    createParticleAtPosition(x, y);
  }
}

void Simulation::updateMousePosition(int x, int y) {
  physics.setMousePosition(static_cast<float>(x), static_cast<float>(y));
}

void Simulation::disableMouseRepulsion() { physics.disableMouseRepulsion(); }

void Simulation::toggleGravity() { physics.toggleGravity(); }

void Simulation::toggleMultithreading() {
  multithreadingEnabled = !multithreadingEnabled;
  physics.setMultithreadingEnabled(multithreadingEnabled);
}

void Simulation::toggleGrid() {
  gridEnabled = !gridEnabled;
  physics.setGridEnabled(gridEnabled);
}

void Simulation::toggleReducedPairwiseComparisons() {
  reducedPairwiseComparisonsEnabled = !reducedPairwiseComparisonsEnabled;
  // Note: PhysicsEngine currently handles this implicitly via SpatialHash or
  // brute force
}

float Simulation::getFrameRate() const { return frameRate; }

int Simulation::getParticleCount() const {
  return static_cast<int>(particles.count);
}

Vec2 Simulation::getAverageVelocity() const {
  if (particles.count == 0)
    return {0, 0};

  float sumX = 0;
  float sumY = 0;
  for (size_t i = 0; i < particles.count; ++i) {
    sumX += particles.velX[i];
    sumY += particles.velY[i];
  }
  return {sumX / particles.count, sumY / particles.count};
}

void Simulation::setParticle(int count) {
  if (count < 0)
    return;

  if (static_cast<size_t>(count) > particles.count) {
    int diff = count - static_cast<int>(particles.count);
    for (int i = 0; i < diff; ++i) {
      createRandomParticle();
    }
  } else {
    particles.count = static_cast<size_t>(count);
  }
}

void Simulation::calculateFrameRate() {
  static auto lastTime = std::chrono::steady_clock::now();
  auto currentTime = std::chrono::steady_clock::now();
  frameCount++;

  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     currentTime - lastTime)
                     .count();
  if (elapsed >= 1000) {
    frameRate = frameCount * 1000.0f / elapsed;
    frameCount = 0;
    lastTime = currentTime;
  }
}

void Simulation::createRandomParticle() {
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_real_distribution<float> distX(0.0f, 1200.0f);
  static std::uniform_real_distribution<float> distY(0.0f, 800.0f);
  static std::uniform_real_distribution<float> distAngle(0.0f, 6.28f);
  static std::uniform_real_distribution<float> distSpeed(0.0f, 50.0f);
  static std::uniform_int_distribution<int> distColor(0, 255);

  static std::uniform_int_distribution<int> distType(0, 100);
  int r = distType(rng);
  int type = TYPE_DEFAULT;
  if (r < 50)
    type = TYPE_DEFAULT; // 50% Default
  else if (r < 80)
    type = TYPE_LIQUID; // 30% Liquid
  else if (r < 90)
    type = TYPE_SAND; // 10% Sand
  else
    type = TYPE_GAS; // 10% Gas

  float x = distX(rng);
  float y = distY(rng);
  float angle = distAngle(rng);
  float speed = distSpeed(rng);
  float vx = std::cos(angle) * speed;
  float vy = std::sin(angle) * speed;

  SDL_Color color;
  if (type == TYPE_LIQUID)
    color = {0, 100, 255, 255}; // Blue
  else if (type == TYPE_SAND)
    color = {237, 201, 175, 255}; // Sand
  else if (type == TYPE_GAS)
    color = {200, 200, 200, 100}; // Grey/White
  else
    color = {static_cast<Uint8>(distColor(rng)),
             static_cast<Uint8>(distColor(rng)),
             static_cast<Uint8>(distColor(rng)), 255};

  particles.addParticle(x, y, vx, vy, 2.0f, 1.0f, type, color);
}

void Simulation::createParticleAtPosition(int x, int y) {
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_real_distribution<float> distAngle(0.0f, 6.28f);
  static std::uniform_real_distribution<float> distSpeed(0.0f, 50.0f);
  static std::uniform_int_distribution<int> distColor(0, 255);

  float angle = distAngle(rng);
  float speed = distSpeed(rng);
  float vx = std::cos(angle) * speed;
  float vy = std::sin(angle) * speed;

  SDL_Color color = {static_cast<Uint8>(distColor(rng)),
                     static_cast<Uint8>(distColor(rng)),
                     static_cast<Uint8>(distColor(rng)), 255};

  particles.addParticle(static_cast<float>(x), static_cast<float>(y), vx, vy,
                        2.0f, 1.0f, TYPE_DEFAULT, color);
}