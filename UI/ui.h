#pragma once
#include <SDL.h>
#include <string>

struct Slider {
    SDL_Rect bar;
    SDL_Rect knob;
    int min, max;
    std::string label;

    Slider(int x, int y, int w, int h, int minValue, int maxValue, std::string labelText);
    void render(SDL_Renderer* renderer);
};
