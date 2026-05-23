#ifndef GUI_H
#define GUI_H

#include "simulation.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Side-panel HUD running on a second SDL window.
//
// Each text label is rendered to a texture once and cached. Hover and toggle
// state are derived from the simulation, not duplicated locally. We graph
// FPS, particle count, and average velocity using a ring buffer that ticks
// once per render.
// ---------------------------------------------------------------------------

class GUI {
public:
  GUI(SDL_Renderer *renderer, TTF_Font *font);
  ~GUI();

  void render(Simulation &sim);
  void handleEvent(const SDL_Event &event, Simulation &sim);

private:
  SDL_Renderer *renderer_;
  TTF_Font     *font_;

  // Cached label textures keyed by string content so we don't rebuild a
  // texture every single frame.
  std::unordered_map<std::string, SDL_Texture *> textCache_;
  SDL_Texture *cachedText(const std::string &text, SDL_Color color);

  SDL_Rect startBtn_, stopBtn_, resetBtn_, gravityBtn_;
  SDL_Rect mtBtn_, gridBtn_, clearBtn_, freezeBtn_;
  SDL_Rect modeBtn_, helpBtn_;
  SDL_Rect inputBox_;
  SDL_Rect fpsRect_, partRect_, modeRect_;

  std::string inputText_ = "1000";
  bool inputActive_ = false;

  // History ring buffers for the graphs.
  std::vector<float> fpsHistory_;
  std::vector<float> countHistory_;
  std::vector<float> velHistory_;
  std::size_t        sampleIdx_ = 0;
  std::size_t        maxSamples_ = 120;

  void renderButton(const SDL_Rect &rect, const std::string &label,
                    bool toggled);
  void renderGraph(const SDL_Rect &rect, const std::vector<float> &data,
                   SDL_Color color, const std::string &title, float maxOverride = 0.0f);
  void drawText(const std::string &text, int x, int y, SDL_Color c);

  void updateMetrics(const Simulation &sim);

  bool pointIn(int x, int y, const SDL_Rect &r);
};

#endif
