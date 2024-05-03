#include "ui.h"

Slider::Slider(int x, int y, int w, int h, int minValue, int maxValue, std::string labelText)
    : min(minValue), max(maxValue), label(labelText) {
    bar = {x, y, w, h};
    knob = {x, y - h/2, h, h};  // Knob is centered on the bar vertically
}

void Slider::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &bar);
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
    SDL_RenderFillRect(renderer, &knob);
}
