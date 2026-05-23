#include "collisions.h"
#include "config.h"

#include <algorithm>
#include <cmath>

namespace collisions {

// One-sided collision response: when looking at pair (i, j) we only
// accumulate into i's scratch slot. j will reciprocate when it processes its
// own neighbourhood. This makes the pass parallel-safe — every thread only
// writes to indices it owns.
//
// We resolve overlap by moving i away from j by half the penetration depth
// (other half handled by j's pass), then apply an impulse along the contact
// normal with restitution + Coulomb-ish friction.
void resolveBand(ParticleSystem &p,
                 const SpatialHash &hash,
                 const std::vector<std::uint32_t> &sortedIndices,
                 std::size_t begin, std::size_t end) {
  const float bias = cfg::POSITION_BIAS;
  const float e    = cfg::COLLISION_RESTITUTION;
  const float mu   = cfg::COLLISION_FRICTION;

  for (std::size_t i = begin; i < end; ++i) {
    const float pxi = p.posX[i];
    const float pyi = p.posY[i];
    const float vxi = p.velX[i];
    const float vyi = p.velY[i];
    const float ri  = p.radius[i];
    const float wi  = p.invMass[i];      // inverse mass
    const auto  ti  = static_cast<ParticleType>(p.type[i]);

    int cx = hash.cellIndexX(pxi);
    int cy = hash.cellIndexY(pyi);

    float pushX = 0.0f, pushY = 0.0f;
    float dvX   = 0.0f, dvY   = 0.0f;

    for (int dy = -1; dy <= 1; ++dy) {
      for (int dx = -1; dx <= 1; ++dx) {
        const auto &cell = hash.getCell(cx + dx, cy + dy);
        for (std::uint32_t k = 0; k < cell.count; ++k) {
          std::uint32_t j = sortedIndices[cell.start + k];
          if (j == i) continue;

          float rx = p.posX[j] - pxi;
          float ry = p.posY[j] - pyi;
          float dist2 = rx * rx + ry * ry;
          float rSum  = ri + p.radius[j];
          float rSum2 = rSum * rSum;
          if (dist2 >= rSum2 || dist2 < 1e-6f) continue;

          float dist = std::sqrt(dist2);
          float invDist = 1.0f / dist;
          float nx = rx * invDist;   // points i -> j
          float ny = ry * invDist;
          float overlap = rSum - dist;

          // Mass-weighted partition: lighter particle (larger invMass)
          // gets pushed more. If both are kinematic (stones), skip.
          float wj = p.invMass[j];
          float wSum = wi + wj;
          if (wSum <= 0.0f) continue;

          float share = bias * (wi / wSum);

          // Liquid cohesion: don't push as hard, lets droplets coalesce.
          float pushScale = 1.0f;
          if (ti == TYPE_LIQUID && p.type[j] == TYPE_LIQUID) {
            pushScale = cfg::LIQUID_COHESION;
          }

          pushX -= nx * overlap * share * pushScale;
          pushY -= ny * overlap * share * pushScale;

          // Velocity exchange along normal.
          float vRelX = p.velX[j] - vxi;
          float vRelY = p.velY[j] - vyi;
          float vN    = vRelX * nx + vRelY * ny;
          if (vN < 0.0f) {
            // Particles separating — no impulse needed.
            continue;
          }
          // Impulse magnitude such that the relative normal velocity flips
          // sign and scales by restitution.
          float jImp = (1.0f + e) * vN / wSum;
          dvX += nx * jImp * wi;
          dvY += ny * jImp * wi;

          // Tangential friction (Coulomb cone).
          float tx = -ny, ty = nx;
          float vT = vRelX * tx + vRelY * ty;
          float frictionScale = mu;
          if (ti == TYPE_SAND || p.type[j] == TYPE_SAND) {
            frictionScale = cfg::SAND_FRICTION_COEF;
          }
          float fImp = vT * frictionScale / wSum;
          dvX += tx * fImp * wi;
          dvY += ty * fImp * wi;
        }
      }
    }

    // Write to scratch slots in the SoA. accX/accY are reused as a
    // position-correction scratch here (forces have already been
    // converted into velocities for this substep, so the slots are free).
    p.accX[i] = pushX;
    p.accY[i] = pushY;
    // We accumulate the velocity delta directly — safe because we only
    // touch our own index i.
    p.velX[i] += dvX;
    p.velY[i] += dvY;
  }
}

void applyWorldBounds(ParticleSystem &p, std::size_t begin, std::size_t end) {
  const float w = cfg::WORLD_WIDTH;
  const float h = cfg::WORLD_HEIGHT;
  const float r = cfg::BOUNDARY_RESTITUTION;

  for (std::size_t i = begin; i < end; ++i) {
    float radius = p.radius[i];
    if (p.posX[i] < radius) {
      p.posX[i] = radius;
      if (p.velX[i] < 0.0f) p.velX[i] = -p.velX[i] * r;
    } else if (p.posX[i] > w - radius) {
      p.posX[i] = w - radius;
      if (p.velX[i] > 0.0f) p.velX[i] = -p.velX[i] * r;
    }
    if (p.posY[i] < radius) {
      p.posY[i] = radius;
      if (p.velY[i] < 0.0f) p.velY[i] = -p.velY[i] * r;
    } else if (p.posY[i] > h - radius) {
      p.posY[i] = h - radius;
      if (p.velY[i] > 0.0f) p.velY[i] = -p.velY[i] * r;
    }
  }
}

} // namespace collisions
