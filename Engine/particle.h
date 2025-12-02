#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <simd/simd.h>
#endif

// Simple Vec2 for non-critical paths
struct Vec2 {
  float x, y;
  Vec2(float x = 0, float y = 0) : x(x), y(y) {}
  float magnitude() const { return std::sqrt(x * x + y * y); }
  Vec2 operator-(const Vec2 &other) const { return {x - other.x, y - other.y}; }
  Vec2 operator*(float s) const { return {x * s, y * s}; }
  Vec2 operator+(const Vec2 &other) const { return {x + other.x, y + other.y}; }
};

enum ParticleType {
  TYPE_DEFAULT = 0,
  TYPE_LIQUID,
  TYPE_SAND,
  TYPE_GAS,
  TYPE_STONE
};

class ParticleSystem {
public:
  // Structure of Arrays (SoA) data layout
  std::vector<float> posX;
  std::vector<float> posY;
  std::vector<float> velX;
  std::vector<float> velY;
  std::vector<float> accX; // For Verlet integration
  std::vector<float> accY;

  std::vector<uint8_t> colorR;
  std::vector<uint8_t> colorG;
  std::vector<uint8_t> colorB;
  std::vector<uint8_t> colorA;

  std::vector<float> radius;
  std::vector<float> mass;
  std::vector<float> invMass;
  std::vector<int> type;

  size_t count = 0;
  size_t capacity = 0;

  ParticleSystem(size_t initialCapacity = 10000);

  void addParticle(float x, float y, float vx, float vy, float r, float m,
                   int t, SDL_Color c);
  void clear();
  void reserve(size_t newCapacity);
  size_t size() const { return count; }

  // Helper to get a "view" of a particle (for legacy code compatibility if
  // needed) Note: This is slow, use direct array access for performance
  struct ParticleView {
    Vec2 position;
    Vec2 velocity;
    SDL_Color color;
    float radius;
    float mass;
    int type;
  };
  ParticleView getParticle(size_t index) const;

  // Render all particles
  void render(SDL_Renderer *renderer);

private:
  // Texture cache for circles
  SDL_Texture *getCircleTexture(SDL_Renderer *renderer, int radius);
};

#endif