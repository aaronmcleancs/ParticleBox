#include "forces.h"
#include "config.h"

#include <cmath>

namespace forces {

void zeroAccelerations(ParticleSystem &p, std::size_t begin, std::size_t end) {
  for (std::size_t i = begin; i < end; ++i) {
    p.accX[i] = 0.0f;
    p.accY[i] = 0.0f;
  }
}

void applyGravity(ParticleSystem &p, const InputState &in,
                  std::size_t begin, std::size_t end) {
  if (!in.gravityEnabled) return;
  const float gx = in.gravity.x;
  const float gy = in.gravity.y;
  for (std::size_t i = begin; i < end; ++i) {
    const auto t = static_cast<ParticleType>(p.type[i]);
    if (t == TYPE_STONE) continue;
    if (t == TYPE_GAS) {
      p.accX[i] += gx * cfg::GAS_BUOYANCY_MULT;
      p.accY[i] += gy * cfg::GAS_BUOYANCY_MULT;
    } else {
      p.accX[i] += gx;
      p.accY[i] += gy;
    }
  }
}

void applyWind(ParticleSystem &p, const InputState &in,
               std::size_t begin, std::size_t end) {
  if (in.wind.x == 0.0f && in.wind.y == 0.0f) return;
  const float wx = in.wind.x, wy = in.wind.y;
  for (std::size_t i = begin; i < end; ++i) {
    if (p.type[i] == TYPE_STONE) continue;
    p.accX[i] += wx;
    p.accY[i] += wy;
  }
}

void applyMouseField(ParticleSystem &p, const InputState &in,
                     std::size_t begin, std::size_t end) {
  if (!in.leftDown) return;
  const MouseMode m = in.mode;
  if (m != MouseMode::Attract && m != MouseMode::Repel &&
      m != MouseMode::Vortex  && m != MouseMode::Drag) return;

  const float radius = in.brushRadius;
  const float r2     = radius * radius;
  const float mx = in.mousePos.x, my = in.mousePos.y;

  for (std::size_t i = begin; i < end; ++i) {
    if (p.type[i] == TYPE_STONE) continue;
    float dx = p.posX[i] - mx;
    float dy = p.posY[i] - my;
    float d2 = dx * dx + dy * dy;
    if (d2 > r2 || d2 < 1e-4f) continue;

    float d = std::sqrt(d2);
    float falloff = 1.0f - d / radius;          // 1 at centre, 0 at edge
    float invD = 1.0f / d;
    float nx = dx * invD, ny = dy * invD;

    switch (m) {
      case MouseMode::Repel: {
        float s = cfg::MOUSE_REPEL_STRENGTH * falloff;
        p.accX[i] += nx * s;
        p.accY[i] += ny * s;
        break;
      }
      case MouseMode::Attract: {
        float s = cfg::MOUSE_ATTRACT_STRENGTH * falloff;
        p.accX[i] -= nx * s;
        p.accY[i] -= ny * s;
        break;
      }
      case MouseMode::Vortex: {
        // Tangent vector (perpendicular to radial) -> swirl
        float s = cfg::MOUSE_VORTEX_STRENGTH * falloff;
        p.accX[i] += -ny * s;
        p.accY[i] +=  nx * s;
        // Tiny pull inward keeps the vortex coherent
        p.accX[i] -= nx * s * 0.20f;
        p.accY[i] -= ny * s * 0.20f;
        break;
      }
      case MouseMode::Drag: {
        // Spring-toward-cursor with velocity damping. Acts like a
        // soft tractor beam.
        float k = cfg::MOUSE_DRAG_STIFFNESS;
        p.accX[i] -= dx * k;
        p.accY[i] -= dy * k;
        // Critical damping so the particles arrive instead of orbiting.
        p.accX[i] -= p.velX[i] * 4.0f;
        p.accY[i] -= p.velY[i] * 4.0f;
        break;
      }
      default: break;
    }
  }
}

void applyExplosionImpulse(ParticleSystem &p, const InputState &in,
                           std::size_t begin, std::size_t end) {
  if (!in.explodePending) return;
  const float ex = in.explodePosition.x, ey = in.explodePosition.y;
  const float radius = std::max(60.0f, in.brushRadius * 2.0f);
  const float r2 = radius * radius;
  const float strength = cfg::MOUSE_EXPLODE_IMPULSE;

  for (std::size_t i = begin; i < end; ++i) {
    if (p.type[i] == TYPE_STONE) continue;
    float dx = p.posX[i] - ex;
    float dy = p.posY[i] - ey;
    float d2 = dx * dx + dy * dy;
    if (d2 > r2 || d2 < 1e-4f) continue;
    float d = std::sqrt(d2);
    float falloff = 1.0f - d / radius;
    float invD = 1.0f / d;
    // Impulse: instantaneous velocity change (no dt scaling).
    p.velX[i] += dx * invD * strength * falloff * p.invMass[i];
    p.velY[i] += dy * invD * strength * falloff * p.invMass[i];
  }
}

void applyDamping(ParticleSystem &p, std::size_t begin, std::size_t end) {
  for (std::size_t i = begin; i < end; ++i) {
    float d;
    switch (static_cast<ParticleType>(p.type[i])) {
      case TYPE_GAS:    d = cfg::GAS_DAMPING; break;
      case TYPE_LIQUID: d = cfg::LIQUID_DAMPING; break;
      default:          d = cfg::DEFAULT_DAMPING; break;
    }
    p.velX[i] *= d;
    p.velY[i] *= d;
  }
}

} // namespace forces
