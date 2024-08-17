#include "ui.h"
#include "simulation.h"
#include <SDL_ttf.h>
#include <string>


GUI::GUI(SDL_Renderer* renderer, TTF_Font* font) : renderer(renderer), font(font) {
    startButton = {10, 50, 180, 40};
    stopButton = {10, 100, 180, 40};
    resetButton = {10, 150, 180, 40};
    particleCountRect = {10, 10, 180, 30};
    frameRateRect = {200, 10, 180, 30}; 

    SDL_Color textColor = {0, 0, 0, 255}; 
    initTexture(&startTexture, "Start", textColor);
    initTexture(&stopTexture, "Stop", textColor);
    initTexture(&resetTexture, "Reset", textColor);
    particleCountTexture = nullptr;
    frameRateTexture = nullptr;
     gravityButton = {10, 200, 180, 40};
    addParticleButton = {10, 250, 180, 40};
    removeParticleButton = {10, 300, 180, 40};

    initTexture(&gravityTexture, "Toggle Gravity", textColor);
    initTexture(&addParticleTexture, "Add Particle", textColor);
    initTexture(&removeParticleTexture, "Remove Particle", textColor);
}

GUI::~GUI() {
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(stopTexture);
    SDL_DestroyTexture(resetTexture);
    SDL_DestroyTexture(particleCountTexture);
    SDL_DestroyTexture(frameRateTexture);
    SDL_DestroyTexture(gravityTexture);
    SDL_DestroyTexture(addParticleTexture);
    SDL_DestroyTexture(removeParticleTexture);
}

void GUI::render(Simulation& simulation) {
    updateMetricsDisplay(simulation); 


    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255); 
    SDL_RenderFillRect(renderer, &startButton);
    SDL_RenderFillRect(renderer, &stopButton);
    SDL_RenderFillRect(renderer, &resetButton);


    SDL_RenderCopy(renderer, startTexture, NULL, &startButton);
    SDL_RenderCopy(renderer, stopTexture, NULL, &stopButton);
    SDL_RenderCopy(renderer, resetTexture, NULL, &resetButton);


    if (particleCountTexture) {
        SDL_RenderCopy(renderer, particleCountTexture, NULL, &particleCountRect);
    }
    if (frameRateTexture) {
        SDL_RenderCopy(renderer, frameRateTexture, NULL, &frameRateRect);
    }
    SDL_RenderCopy(renderer, gravityTexture, NULL, &gravityButton);
    SDL_RenderCopy(renderer, addParticleTexture, NULL, &addParticleButton);
    SDL_RenderCopy(renderer, removeParticleTexture, NULL, &removeParticleButton);
}

void GUI::updateMetricsDisplay(const Simulation& simulation) {
    std::string particleCountText = "Particles: " + std::to_string(simulation.getParticleCount());
    std::string frameRateText = "FPS: " + std::to_string(simulation.getFrameRate());
    SDL_Color textColor = {0, 0, 0, 255}; // Black text for metrics

    initTexture(&particleCountTexture, particleCountText.c_str(), textColor);

    initTexture(&frameRateTexture, frameRateText.c_str(), textColor);
}

void GUI::initTexture(SDL_Texture** texture, const char* text, SDL_Color color) {
    if (*texture) {
        SDL_DestroyTexture(*texture); 
    }
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void GUI::handleEvent(SDL_Event& event, Simulation& simulation) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);


        if (x >= startButton.x && x <= startButton.x + startButton.w &&
            y >= startButton.y && y <= startButton.y + startButton.h) {
            simulation.start();
        }

        if (x >= stopButton.x && x <= stopButton.x + stopButton.w &&
            y >= stopButton.y && y <= stopButton.y + stopButton.h) {
            simulation.stop();
        }

        if (x >= resetButton.x && x <= resetButton.x + resetButton.w &&
            y >= resetButton.y && y <= resetButton.y + resetButton.h) {
            simulation.reset();
        }

        if (x >= gravityButton.x && x <= gravityButton.x + gravityButton.w &&
            y >= gravityButton.y && y <= gravityButton.y + gravityButton.h) {
            simulation.toggleGravity();
        }

        if (x >= addParticleButton.x && x <= addParticleButton.x + addParticleButton.w &&
            y >= addParticleButton.y && y <= addParticleButton.y + addParticleButton.h) {
            if (simulation.getParticleCount() < 1000) {
                simulation.addParticle();
            }
        }

        if (x >= removeParticleButton.x && x <= removeParticleButton.x + removeParticleButton.w &&
            y >= removeParticleButton.y && y <= removeParticleButton.y + removeParticleButton.h) {
            simulation.removeParticle();
        }
    }
}
