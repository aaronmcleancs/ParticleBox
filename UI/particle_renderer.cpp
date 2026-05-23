#include "particle_renderer.h"

#include "config.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace {

// Unit-disc vertex positions cached at startup. We multiply by each
// particle's radius when filling the vertex buffer.
struct UnitDisc {
  std::array<float, cfg::RENDER_CIRCLE_VERTS * 2> xy;
  UnitDisc() {
    constexpr int V = cfg::RENDER_CIRCLE_VERTS;
    for (int i = 0; i < V; ++i) {
      float a = (static_cast<float>(i) / V) * 6.28318530718f;
      xy[i * 2 + 0] = std::cos(a);
      xy[i * 2 + 1] = std::sin(a);
    }
  }
};

const UnitDisc &unitDisc() {
  static const UnitDisc disc;
  return disc;
}

// Speed-based RGB tint. Cool blue at rest, hot orange at high speed.
inline void speedToRGB(float speedSq, SDL_Color base,
                       Uint8 &outR, Uint8 &outG, Uint8 &outB) {
  // Saturate around speed 80 (= speedSq 6400).
  float t = std::min(1.0f, speedSq / 6400.0f);
  // Blend from base colour at rest -> hot at high speed.
  Uint8 hotR = 255, hotG = 90, hotB = 30;
  outR = static_cast<Uint8>(base.r * (1.0f - t) + hotR * t);
  outG = static_cast<Uint8>(base.g * (1.0f - t) + hotG * t);
  outB = static_cast<Uint8>(base.b * (1.0f - t) + hotB * t);
}

} // namespace

namespace ParticleRenderer {

void draw(SDL_Renderer *renderer, const ParticleSystem &p) {
  if (!renderer || p.count == 0) return;

  constexpr int V    = cfg::RENDER_CIRCLE_VERTS;
  constexpr int TRIS = V - 2;       // fan triangulation
  const auto &disc   = unitDisc();

  // Per particle: V vertices, TRIS*3 indices.
  static thread_local std::vector<SDL_Vertex> verts;
  static thread_local std::vector<int>        idx;
  verts.clear();
  idx.clear();
  verts.reserve(p.count * V);
  idx.reserve(p.count * TRIS * 3);

  for (std::size_t i = 0; i < p.count; ++i) {
    float cx = p.posX[i];
    float cy = p.posY[i];
    float r  = p.radius[i];

    float speedSq = p.velX[i] * p.velX[i] + p.velY[i] * p.velY[i];
    SDL_Color base{ p.colorR[i], p.colorG[i], p.colorB[i], p.colorA[i] };
    Uint8 rC, gC, bC;
    speedToRGB(speedSq, base, rC, gC, bC);
    SDL_Color col{ rC, gC, bC, p.colorA[i] };

    int baseV = static_cast<int>(verts.size());

    for (int v = 0; v < V; ++v) {
      SDL_Vertex sv;
      sv.position.x = cx + disc.xy[v * 2 + 0] * r;
      sv.position.y = cy + disc.xy[v * 2 + 1] * r;
      sv.color      = col;
      sv.tex_coord.x = 0.0f;
      sv.tex_coord.y = 0.0f;
      verts.push_back(sv);
    }
    // Triangle fan via index list: (0,k,k+1) for k in 1..V-2
    for (int k = 1; k < V - 1; ++k) {
      idx.push_back(baseV + 0);
      idx.push_back(baseV + k);
      idx.push_back(baseV + k + 1);
    }
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderGeometry(renderer,
                     /*texture*/ nullptr,
                     verts.data(),
                     static_cast<int>(verts.size()),
                     idx.data(),
                     static_cast<int>(idx.size()));
}

void drawBrush(SDL_Renderer *renderer, int x, int y, float radius,
               SDL_Color color) {
  if (!renderer) return;
  // Hollow circle outline for the brush indicator.
  constexpr int SEG = 48;
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  float px = static_cast<float>(x) + radius;
  float py = static_cast<float>(y);
  for (int i = 1; i <= SEG; ++i) {
    float a = (static_cast<float>(i) / SEG) * 6.28318530718f;
    float nx = static_cast<float>(x) + std::cos(a) * radius;
    float ny = static_cast<float>(y) + std::sin(a) * radius;
    SDL_RenderDrawLine(renderer, static_cast<int>(px), static_cast<int>(py),
                       static_cast<int>(nx), static_cast<int>(ny));
    px = nx; py = ny;
  }
}

} // namespace ParticleRenderer
