#include "simulation.h"

#include "particle_renderer.h"

#include <algorithm>
#include <cmath>

Simulation::Simulation()
    : particles_(cfg::INITIAL_CAPACITY),
      rng_(std::random_device{}()) {
  fpsStart_ = std::chrono::steady_clock::now();
  physics_.setMultithreadingEnabled(input_.multithreadEnabled);
  physics_.setGridEnabled(input_.gridEnabled);
}

void Simulation::start() { running_ = true; }
void Simulation::stop()  { running_ = false; }

void Simulation::reset(int particleCount) {
  particles_.clear();
  particles_.reserve(static_cast<std::size_t>(std::max(particleCount, 0)));
  for (int i = 0; i < particleCount; ++i) {
    createRandomParticle();
  }
}

void Simulation::clearParticles() {
  particles_.clear();
}

void Simulation::freezeAll() {
  std::fill(particles_.velX.begin(),
            particles_.velX.begin() + particles_.count, 0.0f);
  std::fill(particles_.velY.begin(),
            particles_.velY.begin() + particles_.count, 0.0f);
}

void Simulation::update(float frameDt) {
  if (!running_) return;
  if (input_.paused) {
    frameRate_ = 0.0f;
    return;
  }

  auto t0 = std::chrono::steady_clock::now();

  // Forward toggles to physics in case they changed since last frame.
  physics_.setMultithreadingEnabled(input_.multithreadEnabled);
  physics_.setGridEnabled(input_.gridEnabled);

  physics_.update(particles_, input_, frameDt);

  // Consume one-shot triggers
  if (input_.explodePending) {
    input_.explodePending = false;
  }

  auto t1 = std::chrono::steady_clock::now();
  float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
  // Exponential moving average
  avgUpdateMs_ = avgUpdateMs_ * 0.92f + ms * 0.08f;

  // FPS counter (windowed)
  ++frameCount_;
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     t1 - fpsStart_).count();
  if (elapsed >= 500) {
    frameRate_ = frameCount_ * 1000.0f / static_cast<float>(elapsed);
    frameCount_ = 0;
    fpsStart_ = t1;
  }
}

void Simulation::render(SDL_Renderer *renderer) {
  ParticleRenderer::draw(renderer, particles_);
}

void Simulation::spawnBrush(int x, int y, int count, float brushRadius,
                            ParticleType t) {
  std::uniform_real_distribution<float> dr(0.0f, brushRadius);
  std::uniform_real_distribution<float> da(0.0f, 6.28318f);
  std::uniform_real_distribution<float> dv(-10.0f, 10.0f);

  for (int i = 0; i < count; ++i) {
    float r = dr(rng_) * std::sqrt(dr(rng_) / brushRadius); // bias toward centre
    float a = da(rng_);
    float px = static_cast<float>(x) + std::cos(a) * r;
    float py = static_cast<float>(y) + std::sin(a) * r;
    spawnAt(px, py, t);
    // Add a little jitter to velocity for natural-looking emissions
    auto &p = particles_;
    p.velX[p.count - 1] += dv(rng_);
    p.velY[p.count - 1] += dv(rng_);
  }
}

void Simulation::eraseBrush(int x, int y, float brushRadius) {
  float r2 = brushRadius * brushRadius;
  std::size_t i = 0;
  while (i < particles_.count) {
    float dx = particles_.posX[i] - x;
    float dy = particles_.posY[i] - y;
    if (dx * dx + dy * dy <= r2) {
      particles_.removeSwap(i);
    } else {
      ++i;
    }
  }
}

void Simulation::spawnAt(float x, float y, ParticleType t) {
  if (x < 0.0f || x > cfg::WORLD_WIDTH ||
      y < 0.0f || y > cfg::WORLD_HEIGHT) return;
  float mass = cfg::DEFAULT_MASS;
  float radius = cfg::DEFAULT_RADIUS;
  if (t == TYPE_STONE)   { mass = 8.0f; radius = cfg::DEFAULT_RADIUS * 1.6f; }
  else if (t == TYPE_GAS){ mass = 0.5f; }
  else if (t == TYPE_LIQUID) { mass = 0.9f; }
  particles_.add(x, y, 0.0f, 0.0f, radius, mass, t, particleTypeColor(t));
}

void Simulation::triggerExplosion(float x, float y) {
  input_.explodePending = true;
  input_.explodePosition = { x, y };
}

void Simulation::toggleGravity() {
  input_.gravityEnabled = !input_.gravityEnabled;
}
void Simulation::toggleMultithreading() {
  input_.multithreadEnabled = !input_.multithreadEnabled;
  physics_.setMultithreadingEnabled(input_.multithreadEnabled);
}
void Simulation::toggleGrid() {
  input_.gridEnabled = !input_.gridEnabled;
  physics_.setGridEnabled(input_.gridEnabled);
}

Vec2 Simulation::getAverageVelocity() const {
  if (particles_.count == 0) return {0.0f, 0.0f};
  double sx = 0.0, sy = 0.0;
  for (std::size_t i = 0; i < particles_.count; ++i) {
    sx += particles_.velX[i];
    sy += particles_.velY[i];
  }
  float inv = 1.0f / static_cast<float>(particles_.count);
  return { static_cast<float>(sx) * inv, static_cast<float>(sy) * inv };
}

void Simulation::createRandomParticle() {
  std::uniform_real_distribution<float> dx(0.0f, cfg::WORLD_WIDTH);
  std::uniform_real_distribution<float> dy(0.0f, cfg::WORLD_HEIGHT);
  std::uniform_real_distribution<float> da(0.0f, 6.28318f);
  std::uniform_real_distribution<float> dv(0.0f, 50.0f);
  std::uniform_int_distribution<int> dt(0, 99);

  int roll = dt(rng_);
  ParticleType t;
  if      (roll < 50) t = TYPE_DEFAULT;
  else if (roll < 75) t = TYPE_LIQUID;
  else if (roll < 88) t = TYPE_SAND;
  else if (roll < 96) t = TYPE_GAS;
  else                t = TYPE_STONE;

  float angle = da(rng_);
  float speed = dv(rng_);
  float vx = std::cos(angle) * speed;
  float vy = std::sin(angle) * speed;

  float mass = cfg::DEFAULT_MASS;
  float radius = cfg::DEFAULT_RADIUS;
  if (t == TYPE_STONE)   { mass = 8.0f; radius = cfg::DEFAULT_RADIUS * 1.6f; vx = vy = 0.0f; }
  else if (t == TYPE_GAS){ mass = 0.5f; }
  else if (t == TYPE_LIQUID) { mass = 0.9f; }

  particles_.add(dx(rng_), dy(rng_), vx, vy, radius, mass, t,
                 particleTypeColor(t));
}
