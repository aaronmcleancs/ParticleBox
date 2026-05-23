#include "help_overlay.h"

#include "particle_renderer.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
SDL_Color kHeading{ 200, 220, 255, 255 };
SDL_Color kKey    { 255, 220, 120, 255 };
SDL_Color kBody   { 220, 220, 220, 255 };
SDL_Color kAccent { 120, 220, 140, 255 };
SDL_Color kDim    { 160, 160, 170, 255 };
} // namespace

HelpOverlay::HelpOverlay(SDL_Renderer *renderer, TTF_Font *font)
    : renderer_(renderer), font_(font) {}

HelpOverlay::~HelpOverlay() = default;

void HelpOverlay::renderText(const std::string &text, int x, int y,
                             SDL_Color color) {
  if (!font_ || text.empty()) return;
  SDL_Surface *surf = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
  if (!surf) return;
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
  SDL_Rect dst{ x, y, surf->w, surf->h };
  SDL_RenderCopy(renderer_, tex, nullptr, &dst);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(surf);
}

void HelpOverlay::drawStatusBar(const InputState &state, float fps,
                                int particleCount, float updateMs) {
  // Background strip
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 140);
  SDL_Rect bar{ 0, 0, 1200, 28 };
  SDL_RenderFillRect(renderer_, &bar);

  char buf[256];
  std::snprintf(buf, sizeof(buf),
                "FPS: %5.1f   particles: %5d   update: %5.2f ms",
                fps, particleCount, updateMs);
  renderText(buf, 8, 4, kAccent);

  std::snprintf(buf, sizeof(buf), "mode: %s   type: %s   brush: %.0f",
                mouseModeName(state.mode),
                particleTypeName(static_cast<ParticleType>(state.spawnType)),
                state.brushRadius);
  renderText(buf, 460, 4, kKey);

  std::string flags;
  if (state.paused)            flags += "[PAUSED] ";
  if (!state.gravityEnabled)   flags += "[no-gravity] ";
  if (!state.gridEnabled)      flags += "[no-grid] ";
  if (!state.multithreadEnabled) flags += "[serial] ";
  if (state.timeScale != 1.0f) {
    char ts[32];
    std::snprintf(ts, sizeof(ts), "[time x%.2f] ", state.timeScale);
    flags += ts;
  }
  if (!flags.empty()) {
    renderText(flags, 880, 4, {255, 200, 90, 255});
  }
}

void HelpOverlay::drawHelp(const InputState & /*state*/) {
  // Translucent panel on the left side of the sim window.
  const int x = 12, y = 40;
  const int w = 360, h = 540;
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer_, 10, 10, 20, 200);
  SDL_Rect bg{ x, y, w, h };
  SDL_RenderFillRect(renderer_, &bg);
  SDL_SetRenderDrawColor(renderer_, 80, 100, 160, 255);
  SDL_RenderDrawRect(renderer_, &bg);

  struct Row { const char *key; const char *desc; SDL_Color color; };
  const Row rows[] = {
    {"Controls",       "(press H to hide)",                kHeading},
    {"",               "",                                  kBody},
    {"Space",          "pause / resume",                    kBody},
    {"G",              "toggle gravity",                    kBody},
    {"R",              "reset",                             kBody},
    {"C",              "clear all particles",               kBody},
    {"F",              "freeze (zero velocities)",          kBody},
    {"M / B",          "toggle multithreading / grid",      kBody},
    {"",               "",                                  kBody},
    {"Brush & spawn",  "",                                  kHeading},
    {"LMB drag",       "act with current tool",             kBody},
    {"Q / E",          "previous / next tool",              kBody},
    {"Wheel",          "brush radius",                      kBody},
    {"PgUp / PgDn",    "spawn count per tick",              kBody},
    {"1..5",           "type: Default Liquid Sand Gas Stone", kBody},
    {"",               "",                                  kBody},
    {"World forces",   "",                                  kHeading},
    {"W A S D (hold)", "wind direction",                    kBody},
    {"Left / Right",   "rotate gravity",                    kBody},
    {"Up / Down",      "weaker / stronger gravity",         kBody},
    {"[ / ]",          "decrease / increase gravity",       kBody},
    {"- / =",          "slow-mo / fast-fwd  (0 = 1.0x)",    kBody},
    {"",               "",                                  kBody},
    {"H",              "toggle this overlay",               kDim},
  };

  int row_y = y + 10;
  for (const auto &r : rows) {
    if (r.key[0] != '\0') {
      renderText(r.key, x + 12, row_y, kKey);
    }
    if (r.desc[0] != '\0') {
      renderText(r.desc, x + 140, row_y, r.color);
    }
    row_y += 19;
  }
}

void HelpOverlay::drawBrush(const InputState &state) {
  SDL_Color c{ 180, 220, 255, 180 };
  switch (state.mode) {
    case MouseMode::Spawn:   c = { 120, 220, 140, 200 }; break;
    case MouseMode::Attract: c = { 100, 200, 255, 200 }; break;
    case MouseMode::Repel:   c = { 255, 100, 100, 200 }; break;
    case MouseMode::Vortex:  c = { 220, 140, 255, 200 }; break;
    case MouseMode::Drag:    c = { 255, 220,  80, 200 }; break;
    case MouseMode::Explode: c = { 255, 140,  40, 200 }; break;
    case MouseMode::Erase:   c = { 255,  80, 120, 200 }; break;
    default: break;
  }
  ParticleRenderer::drawBrush(renderer_,
                              static_cast<int>(state.mousePos.x),
                              static_cast<int>(state.mousePos.y),
                              state.brushRadius, c);
}
