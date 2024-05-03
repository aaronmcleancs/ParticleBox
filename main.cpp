#include <SDL.h>
#include <SDL_ttf.h>
#include "ui.h"
#include "simulation.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Particle Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Simulation simulation;
    Slider gravitySlider(10, 50, 180, 20, -10, 10, "Gravity");
    Slider particleCountSlider(10, 100, 180, 20, 1, 1000, "Particle Count");

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            simulation.handleEvent(event);
        }

        simulation.update(0.016f); // Update the simulation
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        gravitySlider.render(renderer);
        particleCountSlider.render(renderer);
        simulation.render(renderer); // Render the simulation
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
