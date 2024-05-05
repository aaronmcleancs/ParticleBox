#include <SDL.h>
#include <SDL_ttf.h>
#include "simulation.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Particle Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1400, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Simulation simulation;

    // Define GUI elements
    SDL_Rect startButton = {1205, 50, 180, 40}; // Position and size of start button
    SDL_Rect stopButton = {1205, 100, 180, 40}; // Position and size of stop button
    SDL_Rect resetButton = {1205, 150, 180, 40}; // Position and size of reset button

    bool running = true;
    bool simulationRunning = false;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                // Check if buttons are pressed
                if (x >= startButton.x && x <= startButton.x + startButton.w &&
                    y >= startButton.y && y <= startButton.y + startButton.h) {
                    simulationRunning = true;
                } else if (x >= stopButton.x && x <= stopButton.x + stopButton.w &&
                           y >= stopButton.y && y <= stopButton.y + stopButton.h) {
                    simulationRunning = false;
                } else if (x >= resetButton.x && x <= resetButton.x + resetButton.w &&
                           y >= resetButton.y && y <= resetButton.y + resetButton.h) {
                    simulation.reset();
                }
            }
            simulation.handleEvent(event);
        }

        if (simulationRunning) {
            simulation.update(0.016f); // Update the simulation
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        simulation.render(renderer); // Render the simulation

        // Render GUI
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for buttons
        SDL_RenderFillRect(renderer, &startButton);
        SDL_RenderFillRect(renderer, &stopButton);
        SDL_RenderFillRect(renderer, &resetButton);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
