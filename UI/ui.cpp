#include "ui.h"
#include "simulation.h"
#include <SDL_ttf.h>

GUI::GUI(SDL_Renderer* renderer, TTF_Font* font) : renderer(renderer), font(font) {
    // Initialize buttons and textures here
    startButton = {10, 50, 180, 40};
    stopButton = {10, 100, 180, 40};
    resetButton = {10, 150, 180, 40};

    SDL_Color textColor = {0, 0, 0, 255};  // Black text
    SDL_Surface* startSurface = TTF_RenderText_Solid(font, "Start", textColor);
    startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
    SDL_FreeSurface(startSurface);

    SDL_Surface* stopSurface = TTF_RenderText_Solid(font, "Stop", textColor);
    stopTexture = SDL_CreateTextureFromSurface(renderer, stopSurface);
    SDL_FreeSurface(stopSurface);

    SDL_Surface* resetSurface = TTF_RenderText_Solid(font, "Reset", textColor);
    resetTexture = SDL_CreateTextureFromSurface(renderer, resetSurface);
    SDL_FreeSurface(resetSurface);
}

GUI::~GUI() {
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(stopTexture);
    SDL_DestroyTexture(resetTexture);
}

void GUI::render() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color for buttons
    SDL_RenderFillRect(renderer, &startButton);
    SDL_RenderFillRect(renderer, &stopButton);
    SDL_RenderFillRect(renderer, &resetButton);

    // Render text on buttons
    SDL_RenderCopy(renderer, startTexture, NULL, &startButton);
    SDL_RenderCopy(renderer, stopTexture, NULL, &stopButton);
    SDL_RenderCopy(renderer, resetTexture, NULL, &resetButton);
}

void GUI::handleEvent(SDL_Event& event, Simulation& simulation) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        // Check if buttons are pressed
        if (x >= startButton.x && x <= startButton.x + startButton.w &&
            y >= startButton.y && y <= startButton.y + startButton.h) {
            simulation.start();
        } else if (x >= stopButton.x && x <= stopButton.x + stopButton.w &&
                   y >= stopButton.y && y <= stopButton.y + stopButton.h) {
            simulation.stop();
        } else if (x >= resetButton.x && x <= resetButton.x + resetButton.w &&
                   y >= resetButton.y && y <= resetButton.y + resetButton.h) {
            simulation.reset();
        }
    }
}

