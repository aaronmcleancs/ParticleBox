#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "simulation.h"
#include <string>
#include <vector>

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
    SDL_Texture* gravityTexture;
    SDL_Texture* multithreadingTexture; 
    SDL_Texture* gridTexture;             
    SDL_Texture* reducedPairwiseTexture;  
    SDL_Texture* particleCountTexture;
    SDL_Texture* frameRateTexture;
    SDL_Texture* particleCountInputTexture;

    SDL_Rect startButton;
    SDL_Rect stopButton;
    SDL_Rect resetButton;
    SDL_Rect gravityButton;
    SDL_Rect multithreadingButton; 
    SDL_Rect gridButton;            
    SDL_Rect reducedPairwiseButton; 
    SDL_Rect particleCountRect;
    SDL_Rect frameRateRect;
    SDL_Rect particleCountInputRect;

    std::string particleCountInput;
    bool inputActive;

    std::vector<float> fpsHistory;
    std::vector<float> particleCountHistory;
    std::vector<float> averageVelocityHistory; 
    size_t maxGraphSamples;
    size_t currentSampleIndex;

    bool pointInRect(int x, int y, const SDL_Rect& rect);
    void renderButton(const SDL_Rect& rect, SDL_Texture* texture, const std::string& label, bool toggled = false);

    void renderGraph(const SDL_Rect& graphRect, const std::vector<float>& fpsData, const std::vector<float>& particleData, const std::string& title);
    void drawLineGraph(const std::vector<float>& data, const SDL_Rect& graphRect, float maxVal, const SDL_Color& lineColor, int currentIndex);
    
    void renderAverageVelocityGraph(const SDL_Rect& graphRect, const std::vector<float>& avgVelData, const std::string& title);
};

#endif