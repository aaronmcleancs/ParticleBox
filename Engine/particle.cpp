#include "particle.h"
#include <unordered_map>

ParticleSystem::ParticleSystem(size_t initialCapacity) {
  reserve(initialCapacity);
}

void ParticleSystem::reserve(size_t newCapacity) {
  if (newCapacity <= capacity)
    return;

  posX.resize(newCapacity);
  posY.resize(newCapacity);
  velX.resize(newCapacity);
  velY.resize(newCapacity);
  accX.resize(newCapacity);
  accY.resize(newCapacity);

  colorR.resize(newCapacity);
  colorG.resize(newCapacity);
  colorB.resize(newCapacity);
  colorA.resize(newCapacity);

  radius.resize(newCapacity);
  mass.resize(newCapacity);
  invMass.resize(newCapacity);
  type.resize(newCapacity);

  capacity = newCapacity;
}

void ParticleSystem::clear() { count = 0; }

void ParticleSystem::addParticle(float x, float y, float vx, float vy, float r,
                                 float m, int t, SDL_Color c) {
  if (count >= capacity) {
    reserve(capacity * 2 + 1000);
  }

  posX[count] = x;
  posY[count] = y;
  velX[count] = vx;
  velY[count] = vy;
  accX[count] = 0.0f;
  accY[count] = 0.0f;

  colorR[count] = c.r;
  colorG[count] = c.g;
  colorB[count] = c.b;
  colorA[count] = c.a;

  radius[count] = r;
  mass[count] = m;
  invMass[count] = (m != 0.0f) ? 1.0f / m : 0.0f;
  type[count] = t;

  count++;
}

ParticleSystem::ParticleView ParticleSystem::getParticle(size_t index) const {
  if (index >= count)
    return {};
  return {Vec2(posX[index], posY[index]),
          Vec2(velX[index], velY[index]),
          {colorR[index], colorG[index], colorB[index], colorA[index]},
          radius[index],
          mass[index],
          type[index]};
}

// Texture cache
static std::unordered_map<int, SDL_Texture *> circleTextureCache;

SDL_Texture *ParticleSystem::getCircleTexture(SDL_Renderer *renderer,
                                              int radius) {
  if (circleTextureCache.find(radius) != circleTextureCache.end()) {
    return circleTextureCache[radius];
  }

  int diameter = radius * 2;
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, diameter, diameter);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  int cx = radius;
  int cy = radius;
  int x = 0;
  int y = radius;
  int d = 3 - 2 * radius;

  while (x <= y) {
    SDL_RenderDrawLine(renderer, cx - y, cy - x, cx + y, cy - x);
    SDL_RenderDrawLine(renderer, cx - x, cy - y, cx + x, cy - y);
    SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
    SDL_RenderDrawLine(renderer, cx - y, cy + x, cx + y, cy + x);
    if (d < 0) {
      d = d + 4 * x + 6;
    } else {
      d = d + 4 * (x - y) + 10;
      y--;
    }
    x++;
  }

  SDL_SetRenderTarget(renderer, nullptr);
  circleTextureCache[radius] = texture;
  return texture;
}

void ParticleSystem::render(SDL_Renderer *renderer) {
  if (!renderer)
    return;

  // Batch rendering by radius to minimize texture switching if we were using
  // sprites For now, we just iterate. Optimization: Use instanced rendering or
  // points if possible in SDL (SDL_RenderGeometry)

  for (size_t i = 0; i < count; ++i) {
    int r = static_cast<int>(radius[i]);
    if (r <= 0)
      continue;

    SDL_Texture *texture = getCircleTexture(renderer, r);

    // Color modulation
    // Simple speed-based coloring for visual flair
    float speedSq = velX[i] * velX[i] + velY[i] * velY[i];
    float t = std::min(speedSq / 2500.0f, 1.0f); // Max speed 50

    Uint8 red = static_cast<Uint8>(40 + t * (200 - 40));
    Uint8 green = static_cast<Uint8>(40 + t * (20 - 40));
    Uint8 blue = static_cast<Uint8>(255 + t * (20 - 255));

    SDL_SetTextureColorMod(texture, red, green, blue);

    SDL_Rect dst = {static_cast<int>(posX[i] - r),
                    static_cast<int>(posY[i] - r), r * 2, r * 2};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
  }
}