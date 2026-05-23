#ifndef HELP_OVERLAY_H
#define HELP_OVERLAY_H

#include "input_state.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

// ---------------------------------------------------------------------------
// Translucent overlay drawn on the simulation window that documents the
// keyboard controls. Toggled with H.
// ---------------------------------------------------------------------------

class HelpOverlay {
public:
  HelpOverlay(SDL_Renderer *renderer, TTF_Font *font);
  ~HelpOverlay();

  // Status bar (always visible across top): mode, brush, type, paused...
  void drawStatusBar(const InputState &state, float fps, int particleCount,
                     float updateMs);

  // Full keymap (toggle via H).
  void drawHelp(const InputState &state);

  // Brush ring at cursor.
  void drawBrush(const InputState &state);

private:
  SDL_Renderer *renderer_;
  TTF_Font     *font_;

  void renderText(const std::string &text, int x, int y, SDL_Color color);
};

#endif
