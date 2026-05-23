#ifndef PARTICLE_H
#define PARTICLE_H

#include "config.h"
#include "vec2.h"

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Structure-of-Arrays particle store. Splitting the data this way lets the
// physics loops walk contiguous floats which is friendlier to the cache and
// to auto-vectorisation than an array of fat structs would be.
// ---------------------------------------------------------------------------

enum ParticleType : std::uint8_t {
  TYPE_DEFAULT = 0,
  TYPE_LIQUID  = 1,
  TYPE_SAND    = 2,
  TYPE_GAS     = 3,
  TYPE_STONE   = 4,
  TYPE_COUNT
};

const char *particleTypeName(ParticleType t);
SDL_Color   particleTypeColor(ParticleType t);

class ParticleSystem {
public:
  // Kinematics
  std::vector<float> posX, posY;
  std::vector<float> velX, velY;
  std::vector<float> accX, accY;       // accumulator for the force phase

  // Material
  std::vector<float>         radius;
  std::vector<float>         mass;
  std::vector<float>         invMass;  // 0 for kinematic/stone particles
  std::vector<std::uint8_t>  type;     // ParticleType

  // Colour (packed as 4 separate channel arrays so the renderer can
  // build vertex buffers quickly).
  std::vector<std::uint8_t> colorR, colorG, colorB, colorA;

  std::size_t count    = 0;
  std::size_t capacity = 0;

  explicit ParticleSystem(std::size_t initialCapacity = cfg::INITIAL_CAPACITY);

  void reserve(std::size_t newCapacity);
  void clear();
  std::size_t size() const { return count; }

  // Returns the index of the new particle, or count (== failure) if at cap.
  std::size_t add(float x, float y, float vx, float vy,
                  float r, float m, ParticleType t, SDL_Color c);

  // Remove particle at index by swapping with the last; O(1).
  void removeSwap(std::size_t index);

  // Light read-only accessors so external code stays readable.
  Vec2 position(std::size_t i) const { return {posX[i], posY[i]}; }
  Vec2 velocity(std::size_t i) const { return {velX[i], velY[i]}; }
};

#endif
