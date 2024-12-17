#include <SDL.h>
#include <SDL_ttf.h>
#include "simulation.h"
#include "gui.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    TTF_Init();

    SDL_Window* simWindow = SDL_CreateWindow("Particle Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 800, 0);
    SDL_Renderer* simRenderer = SDL_CreateRenderer(simWindow, -1, SDL_RENDERER_ACCELERATED);

    SDL_Window* guiWindow = SDL_CreateWindow("Controls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 400, 760, 0);
    SDL_Renderer* guiRenderer = SDL_CreateRenderer(guiWindow, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("/Users/aaronmclean/Library/Fonts/3270-Regular.ttf", 128);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        SDL_DestroyRenderer(simRenderer);
        SDL_DestroyWindow(simWindow);
        SDL_DestroyRenderer(guiRenderer);
        SDL_DestroyWindow(guiWindow);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    Simulation simulation;
    GUI gui(guiRenderer, font);

    bool running = true;
    SDL_Event event;
    
    Uint32 lastSpawnTime = 0;
    const Uint32 spawnInterval = 50;
    bool mouseDown = false;
    int mouseX = 0, mouseY = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseDown = true;
                    mouseX = event.button.x;
                    mouseY = event.button.y;
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseDown = false;
                }
            } else if (event.type == SDL_MOUSEMOTION && mouseDown) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }
            
            gui.handleEvent(event, simulation);
        }

        Uint32 currentTime = SDL_GetTicks();
        if (mouseDown && currentTime - lastSpawnTime >= spawnInterval) {
            simulation.spawnParticlesAtMouse(mouseX, mouseY, 1);
            lastSpawnTime = currentTime;
        }

        simulation.update(0.10f);

        SDL_SetRenderDrawColor(simRenderer, 0, 0, 0, 255);
        SDL_RenderClear(simRenderer);
        simulation.render(simRenderer);
        SDL_RenderPresent(simRenderer);

        SDL_SetRenderDrawColor(guiRenderer, 150, 150, 150, 255);
        SDL_RenderClear(guiRenderer);
        gui.render(simulation);
        SDL_RenderPresent(guiRenderer);
    }

    SDL_DestroyRenderer(simRenderer);
    SDL_DestroyWindow(simWindow);
    SDL_DestroyRenderer(guiRenderer);
    SDL_DestroyWindow(guiWindow);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
