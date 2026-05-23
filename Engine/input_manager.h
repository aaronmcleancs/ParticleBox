#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "input_state.h"

#include <SDL2/SDL.h>
#include <array>

class Simulation;

// ---------------------------------------------------------------------------
// Owns the source-of-truth for which keys are currently held and dispatches
// SDL events into the shared InputState (and a few simulation toggles).
//
// Continuous controls (WASD wind, arrow-key gravity tilt) are handled in
// updateContinuous() once per frame so they don't depend on event rate.
// ---------------------------------------------------------------------------

class InputManager {
public:
  explicit InputManager(InputState &state) : state_(state) {}

  // Called for every SDL event sourced from the simulation window.
  // Returns true if the event was consumed.
  bool handleEvent(const SDL_Event &ev, Simulation &sim, int simWindowId);

  // Called once per frame after pumping events. Reads SDL keyboard state
  // to drive continuous forces (wind, gravity tilt, brush size hold).
  void updateContinuous(Simulation &sim, float frameDt);

private:
  InputState &state_;
  bool        showFps_ = true;
};

#endif
