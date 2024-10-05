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

    particleCountInputRect = {10, 250, 180, 40};
    inputActive = false;
    particleCountInput = "100";

    initTexture(&gravityTexture, "Toggle Gravity", textColor);
    particleCountInputTexture = nullptr;
}

GUI::~GUI() {
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(stopTexture);
    SDL_DestroyTexture(resetTexture);
    SDL_DestroyTexture(particleCountTexture);
    SDL_DestroyTexture(frameRateTexture);
    SDL_DestroyTexture(gravityTexture);
    SDL_DestroyTexture(particleCountInputTexture);
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

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &particleCountInputRect);

    SDL_Color textColor = {0, 0, 0, 255};
    initTexture(&particleCountInputTexture, particleCountInput.c_str(), textColor);

    SDL_Rect destRect = particleCountInputRect;
    destRect.x += 5;
    destRect.w -= 10;
    SDL_RenderCopy(renderer, particleCountInputTexture, NULL, &destRect);
    SDL_RenderCopy(renderer, gravityTexture, NULL, &gravityButton);
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
            int newParticleCountReset = std::stoi(particleCountInput);
            simulation.reset(newParticleCountReset);
        }
        if (x >= gravityButton.x && x <= gravityButton.x + gravityButton.w &&
            y >= gravityButton.y && y <= gravityButton.y + gravityButton.h) {
            simulation.toggleGravity();
        }

        if (x >= particleCountInputRect.x && x <= particleCountInputRect.x + particleCountInputRect.w &&
            y >= particleCountInputRect.y && y <= particleCountInputRect.y + particleCountInputRect.h) {
            inputActive = true;
        } else {
            inputActive = false;
        }
    }

    if (event.type == SDL_TEXTINPUT && inputActive) {
        particleCountInput += event.text.text;
    }

    if (event.type == SDL_KEYDOWN && inputActive) {
        if (event.key.keysym.sym == SDLK_BACKSPACE && !particleCountInput.empty()) {
            particleCountInput.pop_back();
        }
        if (event.key.keysym.sym == SDLK_RETURN) {
            try {
                int newParticleCount = std::stoi(particleCountInput);
                simulation.setParticle(newParticleCount); 
            } catch (const std::invalid_argument& e) {
                particleCountInput = "0";
            }
            inputActive = false;
        }
    }
}
