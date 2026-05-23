#include "input_manager.h"

#include "simulation.h"

#include <algorithm>

namespace {
constexpr float kGravityStep      = 1.5f;
constexpr float kGravityRotate    = 0.20f; // radians per second held
constexpr float kBrushZoomFactor  = 1.15f;
constexpr float kBrushMin         = 4.0f;
constexpr float kBrushMax         = 400.0f;
constexpr float kTimeScaleStep    = 0.10f;

ParticleType keyToType(SDL_Keycode k) {
  switch (k) {
    case SDLK_1: return TYPE_DEFAULT;
    case SDLK_2: return TYPE_LIQUID;
    case SDLK_3: return TYPE_SAND;
    case SDLK_4: return TYPE_GAS;
    case SDLK_5: return TYPE_STONE;
    default:     return TYPE_COUNT;
  }
}

MouseMode cycleMode(MouseMode m, int dir) {
  int v = static_cast<int>(m) + dir;
  int n = static_cast<int>(MouseMode::Count);
  v = ((v % n) + n) % n;
  return static_cast<MouseMode>(v);
}
} // namespace

const char *mouseModeName(MouseMode m) {
  switch (m) {
    case MouseMode::Spawn:   return "Spawn";
    case MouseMode::Attract: return "Attract";
    case MouseMode::Repel:   return "Repel";
    case MouseMode::Vortex:  return "Vortex";
    case MouseMode::Drag:    return "Drag";
    case MouseMode::Explode: return "Explode";
    case MouseMode::Erase:   return "Erase";
    default: return "?";
  }
}

bool InputManager::handleEvent(const SDL_Event &ev, Simulation &sim,
                               int simWindowId) {
  switch (ev.type) {

  case SDL_MOUSEMOTION:
    if (ev.motion.windowID == static_cast<Uint32>(simWindowId)) {
      state_.mousePos = { static_cast<float>(ev.motion.x),
                          static_cast<float>(ev.motion.y) };
      return true;
    }
    return false;

  case SDL_MOUSEBUTTONDOWN:
    if (ev.button.windowID != static_cast<Uint32>(simWindowId)) return false;
    state_.mousePos = { static_cast<float>(ev.button.x),
                        static_cast<float>(ev.button.y) };
    if (ev.button.button == SDL_BUTTON_LEFT) {
      state_.leftDown = true;
      if (state_.mode == MouseMode::Explode) {
        state_.explodePending  = true;
        state_.explodePosition = state_.mousePos;
      }
    } else if (ev.button.button == SDL_BUTTON_RIGHT) {
      state_.rightDown = true;
    }
    return true;

  case SDL_MOUSEBUTTONUP:
    if (ev.button.windowID != static_cast<Uint32>(simWindowId)) return false;
    if (ev.button.button == SDL_BUTTON_LEFT)  state_.leftDown  = false;
    if (ev.button.button == SDL_BUTTON_RIGHT) state_.rightDown = false;
    return true;

  case SDL_MOUSEWHEEL:
    if (ev.wheel.windowID != static_cast<Uint32>(simWindowId)) return false;
    if (ev.wheel.y > 0) {
      state_.brushRadius = std::min(kBrushMax, state_.brushRadius * kBrushZoomFactor);
    } else if (ev.wheel.y < 0) {
      state_.brushRadius = std::max(kBrushMin, state_.brushRadius / kBrushZoomFactor);
    }
    return true;

  case SDL_KEYDOWN: {
    if (ev.key.repeat) return false; // discrete actions ignore key auto-repeat
    SDL_Keycode k = ev.key.keysym.sym;

    // Particle type select 1..5
    {
      ParticleType t = keyToType(k);
      if (t != TYPE_COUNT) { state_.spawnType = t; return true; }
    }

    switch (k) {
      case SDLK_SPACE: state_.paused = !state_.paused; return true;
      case SDLK_g:     sim.toggleGravity();
                       state_.gravityEnabled = !state_.gravityEnabled;
                       return true;
      case SDLK_r:     sim.reset(static_cast<int>(sim.getParticleCount()));
                       return true;
      case SDLK_c:     sim.clearParticles(); return true;
      case SDLK_h:     state_.showHelp = !state_.showHelp; return true;
      case SDLK_m:     sim.toggleMultithreading();
                       state_.multithreadEnabled = sim.isMultithreadingEnabled();
                       return true;
      case SDLK_b:     sim.toggleGrid();
                       state_.gridEnabled = sim.isGridEnabled();
                       return true;
      case SDLK_f:     sim.freezeAll(); return true;

      case SDLK_q: state_.mode = cycleMode(state_.mode, -1); return true;
      case SDLK_e: state_.mode = cycleMode(state_.mode, +1); return true;

      case SDLK_LEFTBRACKET:
        state_.gravity.y = std::max(0.0f, state_.gravity.y - kGravityStep);
        return true;
      case SDLK_RIGHTBRACKET:
        state_.gravity.y += kGravityStep;
        return true;

      case SDLK_MINUS:
        state_.timeScale = std::max(0.0f, state_.timeScale - kTimeScaleStep);
        return true;
      case SDLK_EQUALS: // shares physical key with '+'
        state_.timeScale = std::min(4.0f, state_.timeScale + kTimeScaleStep);
        return true;

      case SDLK_0: state_.timeScale = 1.0f; return true;

      case SDLK_PAGEUP:
        state_.spawnPerTick = std::min(50, state_.spawnPerTick + 1);
        return true;
      case SDLK_PAGEDOWN:
        state_.spawnPerTick = std::max(1, state_.spawnPerTick - 1);
        return true;
      default: break;
    }
    return false;
  }

  default: return false;
  }
}

void InputManager::updateContinuous(Simulation & /*sim*/, float frameDt) {
  const Uint8 *keys = SDL_GetKeyboardState(nullptr);

  // WASD wind: held keys apply a directional acceleration to every
  // particle next frame. The wind field is reset every frame so the
  // physics doesn't latch.
  Vec2 w{0.0f, 0.0f};
  if (keys[SDL_SCANCODE_W]) w.y -= 1.0f;
  if (keys[SDL_SCANCODE_S]) w.y += 1.0f;
  if (keys[SDL_SCANCODE_A]) w.x -= 1.0f;
  if (keys[SDL_SCANCODE_D]) w.x += 1.0f;
  if (w.magnitudeSq() > 0.0f) {
    w = w.normalized() * cfg::WIND_FORCE;
  }
  state_.wind = w;

  // Arrow keys tilt gravity smoothly.
  float rot = 0.0f;
  if (keys[SDL_SCANCODE_LEFT])  rot -= kGravityRotate * frameDt;
  if (keys[SDL_SCANCODE_RIGHT]) rot += kGravityRotate * frameDt;
  if (rot != 0.0f) {
    float c = std::cos(rot), s = std::sin(rot);
    Vec2 g = state_.gravity;
    state_.gravity = { g.x * c - g.y * s, g.x * s + g.y * c };
  }
  if (keys[SDL_SCANCODE_UP]) {
    // Gradually shrink gravity magnitude toward 0
    state_.gravity *= (1.0f - 0.6f * frameDt);
  }
  if (keys[SDL_SCANCODE_DOWN]) {
    // Grow gravity magnitude toward 30
    float m = state_.gravity.magnitude();
    Vec2 dir = m > 1e-3f ? state_.gravity * (1.0f / m) : Vec2{0.0f, 1.0f};
    m = std::min(30.0f, m + 12.0f * frameDt);
    state_.gravity = dir * m;
  }
}
