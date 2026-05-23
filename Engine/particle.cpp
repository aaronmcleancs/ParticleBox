#include "particle.h"

ParticleSystem::ParticleSystem(std::size_t initialCapacity) {
  reserve(initialCapacity);
}

void ParticleSystem::reserve(std::size_t newCapacity) {
  if (newCapacity <= capacity) return;

  posX.resize(newCapacity);
  posY.resize(newCapacity);
  velX.resize(newCapacity);
  velY.resize(newCapacity);
  accX.resize(newCapacity);
  accY.resize(newCapacity);

  radius.resize(newCapacity);
  mass.resize(newCapacity);
  invMass.resize(newCapacity);
  type.resize(newCapacity);

  colorR.resize(newCapacity);
  colorG.resize(newCapacity);
  colorB.resize(newCapacity);
  colorA.resize(newCapacity);

  capacity = newCapacity;
}

void ParticleSystem::clear() { count = 0; }

std::size_t ParticleSystem::add(float x, float y, float vx, float vy,
                                float r, float m, ParticleType t,
                                SDL_Color c) {
  if (count >= capacity) {
    reserve(capacity * 2 + 1024);
  }
  std::size_t i = count;
  posX[i] = x; posY[i] = y;
  velX[i] = vx; velY[i] = vy;
  accX[i] = 0.0f; accY[i] = 0.0f;

  radius[i] = r;
  mass[i]   = m;
  invMass[i] = (m > 0.0f && t != TYPE_STONE) ? 1.0f / m : 0.0f;
  type[i]   = static_cast<std::uint8_t>(t);

  colorR[i] = c.r; colorG[i] = c.g; colorB[i] = c.b; colorA[i] = c.a;
  ++count;
  return i;
}

void ParticleSystem::removeSwap(std::size_t i) {
  if (i >= count) return;
  std::size_t last = count - 1;
  if (i != last) {
    posX[i] = posX[last]; posY[i] = posY[last];
    velX[i] = velX[last]; velY[i] = velY[last];
    accX[i] = accX[last]; accY[i] = accY[last];
    radius[i] = radius[last];
    mass[i]   = mass[last];
    invMass[i] = invMass[last];
    type[i]   = type[last];
    colorR[i] = colorR[last]; colorG[i] = colorG[last];
    colorB[i] = colorB[last]; colorA[i] = colorA[last];
  }
  --count;
}

const char *particleTypeName(ParticleType t) {
  switch (t) {
    case TYPE_DEFAULT: return "Default";
    case TYPE_LIQUID:  return "Liquid";
    case TYPE_SAND:    return "Sand";
    case TYPE_GAS:     return "Gas";
    case TYPE_STONE:   return "Stone";
    default:           return "?";
  }
}

SDL_Color particleTypeColor(ParticleType t) {
  switch (t) {
    case TYPE_LIQUID: return {  40, 130, 255, 255 };
    case TYPE_SAND:   return { 237, 201, 175, 255 };
    case TYPE_GAS:    return { 220, 220, 230, 140 };
    case TYPE_STONE:  return { 130, 130, 140, 255 };
    case TYPE_DEFAULT:
    default:          return { 220, 220, 220, 255 };
  }
}
