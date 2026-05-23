#include "font_finder.h"

#include <SDL2/SDL.h>
#include <array>
#include <cstdlib>
#include <sys/stat.h>

namespace {
std::string g_lastFontPath;

bool fileExists(const char *p) {
  struct stat st{};
  return ::stat(p, &st) == 0;
}

// A pragmatic list of well-known system font locations. The order tries the
// most likely match per platform first.
const char *kCandidates[] = {
  // Cross-platform / repo-bundled
  "./assets/fonts/main.ttf",
  "./assets/fonts/Inter-Regular.ttf",

  // macOS
  "/System/Library/Fonts/SFNSMono.ttf",
  "/System/Library/Fonts/Menlo.ttc",
  "/System/Library/Fonts/Monaco.ttf",
  "/System/Library/Fonts/Supplemental/Arial.ttf",
  "/Library/Fonts/Arial.ttf",
  "/System/Library/Fonts/Helvetica.ttc",

  // Linux
  "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
  "/usr/share/fonts/dejavu/DejaVuSans.ttf",
  "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
  "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
  "/usr/share/fonts/TTF/DejaVuSans.ttf",

  // Windows
  "C:/Windows/Fonts/consola.ttf",
  "C:/Windows/Fonts/arial.ttf",
};

} // namespace

const std::string &lastFontPath() { return g_lastFontPath; }

TTF_Font *openSystemFont(int pointSize) {
  // 1. Environment override
  if (const char *env = std::getenv("PARTICLE_FONT")) {
    if (fileExists(env)) {
      TTF_Font *f = TTF_OpenFont(env, pointSize);
      if (f) { g_lastFontPath = env; return f; }
    }
  }

  // 2. Known good locations
  for (const char *p : kCandidates) {
    if (!fileExists(p)) continue;
    TTF_Font *f = TTF_OpenFont(p, pointSize);
    if (f) {
      g_lastFontPath = p;
      return f;
    }
  }

  SDL_Log("font_finder: no usable font found. "
          "Set $PARTICLE_FONT to a .ttf path to override.");
  return nullptr;
}
