#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "simulation.h"

class GUI {
public:
    GUI(SDL_Renderer* renderer, TTF_Font* font);
    ~GUI();
    void render(Simulation& simulation);
    void handleEvent(SDL_Event& event, Simulation& simulation);
    void updateMetricsDisplay(const Simulation& simulation);
    void initTexture(SDL_Texture** texture, const char* text, SDL_Color color);

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* startTexture;
    SDL_Texture* stopTexture;
    SDL_Texture* resetTexture;
    SDL_Texture* particleCountTexture;
    SDL_Texture* frameRateTexture;
    SDL_Texture* gravityTexture;
    SDL_Texture* particleCountInputTexture;
    SDL_Texture* frameRateCapTexture;  // New texture for frame rate cap button
    SDL_Rect startButton;
    SDL_Rect stopButton;
    SDL_Rect resetButton;
    SDL_Rect particleCountRect;
    SDL_Rect frameRateRect;
    SDL_Rect gravityButton;
    SDL_Rect particleCountInputRect;
    SDL_Rect frameRateCapToggleButton; // New SDL_Rect for the frame rate cap toggle button
    std::string particleCountInput;
    bool inputActive;
    bool frameRateCapEnabled; // New variable to hold the state of the frame rate cap
};

#endif
