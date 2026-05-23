// ParticleSimulator - main entry point.
//
// Two SDL windows:
//   * the simulation window (1200x800) shows particles plus an optional
//     translucent help overlay.
//   * a side-by-side controls window (400x800) exposes buttons, toggles, and
//     metric graphs.
//
// Keyboard focus is on the simulation window; the side panel is purely a
// supplementary HUD with mouse buttons.

#include "font_finder.h"
#include "gui.h"
#include "help_overlay.h"
#include "input_manager.h"
#include "particle_renderer.h"
#include "simulation.h"
#include "test.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <cstdio>
#include <string>

namespace {

constexpr int kSimW = 1200, kSimH = 800;
constexpr int kGuiW =  400, kGuiH = 800;

// Continuous spawn/erase tick (ms between brush applications).
constexpr Uint32 kBrushInterval = 24;

bool init(SDL_Window **sw, SDL_Renderer **sr,
          SDL_Window **gw, SDL_Renderer **gr) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }
  if (TTF_Init() != 0) {
    std::fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
    return false;
  }

  *sw = SDL_CreateWindow("Particle Simulator",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         kSimW, kSimH, SDL_WINDOW_SHOWN);
  *sr = SDL_CreateRenderer(*sw, -1,
                           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  *gw = SDL_CreateWindow("Controls",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         kGuiW, kGuiH, SDL_WINDOW_SHOWN);
  *gr = SDL_CreateRenderer(*gw, -1, SDL_RENDERER_ACCELERATED);

  if (!*sw || !*sr || !*gw || !*gr) {
    std::fprintf(stderr, "Window/renderer create failed: %s\n", SDL_GetError());
    return false;
  }
  SDL_StartTextInput();
  return true;
}

} // namespace

int main(int argc, char *argv[]) {

  bool runTests = false;
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-test") { runTests = true; break; }
  }
  if (runTests) {
    runPerformanceTests();
    return 0;
  }

  SDL_Window *simWin = nullptr, *guiWin = nullptr;
  SDL_Renderer *simRen = nullptr, *guiRen = nullptr;
  if (!init(&simWin, &simRen, &guiWin, &guiRen)) {
    return 1;
  }

  TTF_Font *font = openSystemFont(16);
  if (!font) {
    // Continue silently; the help overlay/HUD will just be invisible.
    std::fprintf(stderr,
                 "Warning: continuing without a font. Set $PARTICLE_FONT.\n");
  }

  Simulation simulation;
  simulation.reset(1000);
  simulation.start();

  GUI gui(guiRen, font);
  HelpOverlay overlay(simRen, font);
  InputManager inputs(simulation.input());

  Uint32 lastBrushTime = 0;
  Uint32 lastFrameMs   = SDL_GetTicks();

  int simWindowId = static_cast<int>(SDL_GetWindowID(simWin));
  int guiWindowId = static_cast<int>(SDL_GetWindowID(guiWin));

  bool running = true;
  SDL_Event ev;
  while (running) {
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) { running = false; break; }
      // Window close X
      if (ev.type == SDL_WINDOWEVENT &&
          ev.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false; break;
      }

      // Try input manager first (sim-window events)
      bool consumed = inputs.handleEvent(ev, simulation, simWindowId);

      // Pass GUI window mouse/keyboard events through.
      if (!consumed) {
        if ((ev.type == SDL_MOUSEBUTTONDOWN ||
             ev.type == SDL_MOUSEBUTTONUP   ||
             ev.type == SDL_MOUSEMOTION) &&
            ev.button.windowID == static_cast<Uint32>(guiWindowId)) {
          gui.handleEvent(ev, simulation);
        } else if (ev.type == SDL_TEXTINPUT || ev.type == SDL_KEYDOWN) {
          gui.handleEvent(ev, simulation);
        }
      }
    }

    // Continuous input (WASD wind, arrow tilt)
    Uint32 nowMs = SDL_GetTicks();
    float  dtMs  = static_cast<float>(nowMs - lastFrameMs);
    lastFrameMs = nowMs;
    inputs.updateContinuous(simulation, dtMs * 0.001f);

    // While LMB is held, the spawn/erase brushes apply at a steady rate
    // (force/attract/etc. modes are handled in physics every step).
    if (simulation.input().leftDown && nowMs - lastBrushTime >= kBrushInterval) {
      const InputState &st = simulation.input();
      switch (st.mode) {
        case MouseMode::Spawn:
          simulation.spawnBrush(static_cast<int>(st.mousePos.x),
                                static_cast<int>(st.mousePos.y),
                                st.spawnPerTick, st.brushRadius,
                                static_cast<ParticleType>(st.spawnType));
          break;
        case MouseMode::Erase:
          simulation.eraseBrush(static_cast<int>(st.mousePos.x),
                                static_cast<int>(st.mousePos.y),
                                st.brushRadius);
          break;
        default:
          // Force fields are handled inside the physics step
          break;
      }
      lastBrushTime = nowMs;
    }

    // Step simulation
    simulation.update(cfg::DT_DEFAULT);

    // --- Render sim window ---
    SDL_SetRenderDrawColor(simRen, 8, 9, 14, 255);
    SDL_RenderClear(simRen);
    simulation.render(simRen);
    if (font) {
      overlay.drawStatusBar(simulation.input(),
                            simulation.getFrameRate(),
                            simulation.getParticleCount(),
                            simulation.getAvgUpdateMs());
      if (simulation.input().showHelp) {
        overlay.drawHelp(simulation.input());
      }
      overlay.drawBrush(simulation.input());
    }
    SDL_RenderPresent(simRen);

    // --- Render GUI window ---
    SDL_SetRenderDrawColor(guiRen, 22, 24, 30, 255);
    SDL_RenderClear(guiRen);
    if (font) gui.render(simulation);
    SDL_RenderPresent(guiRen);
  }

  SDL_StopTextInput();
  if (font) TTF_CloseFont(font);
  SDL_DestroyRenderer(simRen);
  SDL_DestroyWindow(simWin);
  SDL_DestroyRenderer(guiRen);
  SDL_DestroyWindow(guiWin);
  TTF_Quit();
  SDL_Quit();
  return 0;
}
