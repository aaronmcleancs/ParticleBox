#ifndef FONT_FINDER_H
#define FONT_FINDER_H

#include <SDL2/SDL_ttf.h>
#include <string>

// ---------------------------------------------------------------------------
// Hunts through a list of common font paths and opens the first that exists.
// Returns nullptr if nothing was found, in which case the caller should fall
// back to running without text.
// ---------------------------------------------------------------------------

TTF_Font *openSystemFont(int pointSize);

// Optional override: read from $PARTICLE_FONT env var first.
const std::string &lastFontPath();

#endif
