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
    SDL_Rect startButton;
    SDL_Rect stopButton;
    SDL_Rect resetButton;
    SDL_Rect particleCountRect;
    SDL_Rect frameRateRect;
    SDL_Rect gravityButton;
    SDL_Rect particleCountInputRect;
    std::string particleCountInput;
    bool inputActive;
};


#endif
