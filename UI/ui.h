#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <SDL_ttf.h>

class Simulation;

class GUI {
public:
    GUI(SDL_Renderer* renderer, TTF_Font* font);
    ~GUI();
    void render();
    void handleEvent(SDL_Event& event, Simulation& simulation);
    void updateButtonAppearance(SDL_Rect& button, bool isHovered);

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* startTexture;
    SDL_Texture* stopTexture;
    SDL_Texture* resetTexture;
    SDL_Rect startButton;
    SDL_Rect stopButton;
    SDL_Rect resetButton;
};

#endif
