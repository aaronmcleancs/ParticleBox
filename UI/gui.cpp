#include "gui.h"
#include "simulation.h"
#include <SDL_ttf.h>
#include <string>
#include <algorithm>
#include <cmath>

struct GUIStyle {
    SDL_Color backgroundColor = {34, 34, 34, 255};
    SDL_Color panelColor = {44, 44, 44, 255};
    SDL_Color buttonColor = {64, 64, 64, 255};
    SDL_Color buttonHoverColor = {80, 80, 80, 255};
    SDL_Color textColor = {220, 220, 220, 255};
    SDL_Color inputActiveColor = {60, 60, 60, 255};
    SDL_Color borderColor = {100, 100, 100, 255};

    SDL_Color graphBackground = {30, 30, 30, 255};
    SDL_Color graphLineColor = {0, 180, 0, 255};
    SDL_Color graphAxisColor = {160, 160, 160, 255};
};

static void drawRect(SDL_Renderer* renderer, const SDL_Rect& rect, const SDL_Color& color, bool filled = true) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (filled)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderDrawRect(renderer, &rect);
}

GUI::GUI(SDL_Renderer* renderer, TTF_Font* font) 
    : renderer(renderer), font(font), inputActive(false), particleCountInput("100") 
{
    startButton = {10, 50, 180, 40};
    stopButton = {10, 100, 180, 40};
    resetButton = {10, 150, 180, 40};
    gravityButton = {10, 200, 180, 40};
    particleCountRect = {10, 10, 180, 30};
    frameRateRect = {200, 10, 180, 30};
    particleCountInputRect = {10, 250, 180, 40};

    startTexture = nullptr;
    stopTexture = nullptr;
    resetTexture = nullptr;
    particleCountTexture = nullptr;
    frameRateTexture = nullptr;
    gravityTexture = nullptr;
    particleCountInputTexture = nullptr;

    GUIStyle style;
    initTexture(&startTexture, "Start", style.textColor);
    initTexture(&stopTexture, "Stop", style.textColor);
    initTexture(&resetTexture, "Reset", style.textColor);
    initTexture(&gravityTexture, "Toggle Gravity", style.textColor);

    maxGraphSamples = 60;
    fpsHistory.resize(maxGraphSamples, 0.0f);
    particleCountHistory.resize(maxGraphSamples, 0.0f);
    currentSampleIndex = 0;
}

GUI::~GUI() {
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(stopTexture);
    SDL_DestroyTexture(resetTexture);
    if (particleCountTexture) SDL_DestroyTexture(particleCountTexture);
    if (frameRateTexture) SDL_DestroyTexture(frameRateTexture);
    SDL_DestroyTexture(gravityTexture);
    if (particleCountInputTexture) SDL_DestroyTexture(particleCountInputTexture);
}

void GUI::render(Simulation& simulation) {
    updateMetricsDisplay(simulation);

    GUIStyle style;

    SDL_Rect panelRect = {0, 0, 400, 800};
    drawRect(renderer, panelRect, style.panelColor);

    renderButton(startButton, startTexture, "Start");
    renderButton(stopButton, stopTexture, "Stop");
    renderButton(resetButton, resetTexture, "Reset");
    renderButton(gravityButton, gravityTexture, "Toggle Gravity");

    if (particleCountTexture) {
        SDL_RenderCopy(renderer, particleCountTexture, NULL, &particleCountRect);
    }
    if (frameRateTexture) {
        SDL_Rect fpsBg = frameRateRect;
        fpsBg.x -= 5; fpsBg.y -= 5; fpsBg.w += 10; fpsBg.h += 10;
        drawRect(renderer, fpsBg, {50, 50, 50, 255});
        SDL_RenderCopy(renderer, frameRateTexture, NULL, &frameRateRect);
    }

    SDL_Color inputColor = inputActive ? style.inputActiveColor : style.backgroundColor;
    drawRect(renderer, particleCountInputRect, inputColor);
    initTexture(&particleCountInputTexture, particleCountInput.c_str(), style.textColor);
    SDL_Rect destRect = particleCountInputRect;
    destRect.x += 5;
    destRect.w -= 10;
    SDL_RenderCopy(renderer, particleCountInputTexture, NULL, &destRect);

    SDL_Rect graphRect = {10, 300, 380, 150};
    renderGraph(graphRect, fpsHistory, particleCountHistory, "FPS & Particles Over Time");
}

void GUI::updateMetricsDisplay(const Simulation& simulation) {
    fpsHistory[currentSampleIndex] = simulation.getFrameRate();
    particleCountHistory[currentSampleIndex] = (float)simulation.getParticleCount();
    currentSampleIndex = (currentSampleIndex + 1) % maxGraphSamples;

    GUIStyle style;
    std::string particleCountText = "Particles: " + std::to_string(simulation.getParticleCount());
    std::string frameRateText = "FPS: " + std::to_string((int)std::round(simulation.getFrameRate()));

    initTexture(&particleCountTexture, particleCountText.c_str(), style.textColor);
    initTexture(&frameRateTexture, frameRateText.c_str(), {0, 255, 0, 255}); 
}

void GUI::initTexture(SDL_Texture** texture, const char* text, SDL_Color color) {
    if (*texture) {
        SDL_DestroyTexture(*texture);
        *texture = nullptr;
    }
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void GUI::handleEvent(SDL_Event& event, Simulation& simulation) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);

        if (pointInRect(x, y, startButton)) {
            simulation.start();
        } else if (pointInRect(x, y, stopButton)) {
            simulation.stop();
        } else if (pointInRect(x, y, resetButton)) {
            int newParticleCountReset = std::stoi(particleCountInput.empty() ? "0" : particleCountInput);
            simulation.reset(newParticleCountReset);
        } else if (pointInRect(x, y, gravityButton)) {
            simulation.toggleGravity();
        }

        if (pointInRect(x, y, particleCountInputRect)) {
            inputActive = true;
        } else {
            inputActive = false;
        }

    } else if (event.type == SDL_TEXTINPUT && inputActive) {
        particleCountInput += event.text.text;

    } else if (event.type == SDL_KEYDOWN && inputActive) {
        if (event.key.keysym.sym == SDLK_BACKSPACE && !particleCountInput.empty()) {
            particleCountInput.pop_back();
        }
        if (event.key.keysym.sym == SDLK_RETURN) {
            try {
                int newParticleCount = std::stoi(particleCountInput.empty() ? "0" : particleCountInput);
                simulation.setParticle(newParticleCount); 
            } catch (const std::invalid_argument&) {
                particleCountInput = "0";
            }
            inputActive = false;
        }
    }
}

bool GUI::pointInRect(int x, int y, const SDL_Rect& rect) {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void GUI::renderButton(const SDL_Rect& rect, SDL_Texture* texture, const std::string& label) {
    GUIStyle style;
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    bool hovered = pointInRect(mouseX, mouseY, rect);

    SDL_Color bgColor = hovered ? style.buttonHoverColor : style.buttonColor;
    drawRect(renderer, rect, bgColor);
    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    } else {
        SDL_Texture* tempTex = nullptr;
        initTexture(&tempTex, label.c_str(), style.textColor);
        SDL_RenderCopy(renderer, tempTex, NULL, &rect);
        SDL_DestroyTexture(tempTex);
    }
}

void GUI::renderGraph(const SDL_Rect& graphRect, const std::vector<float>& fpsData, const std::vector<float>& particleData, const std::string& title) {
    GUIStyle style;
    drawRect(renderer, graphRect, style.graphBackground);

    float maxFPS = *std::max_element(fpsData.begin(), fpsData.end());
    float maxParticles = *std::max_element(particleData.begin(), particleData.end());
    float globalMax = std::max(maxFPS, maxParticles);
    if (globalMax < 1.0f) globalMax = 1.0f;

    SDL_SetRenderDrawColor(renderer, style.graphAxisColor.r, style.graphAxisColor.g, style.graphAxisColor.b, style.graphAxisColor.a);
    SDL_RenderDrawLine(renderer, graphRect.x, graphRect.y + graphRect.h - 20, graphRect.x + graphRect.w, graphRect.y + graphRect.h - 20);
    SDL_RenderDrawLine(renderer, graphRect.x + 40, graphRect.y, graphRect.x + 40, graphRect.y + graphRect.h);

    drawLineGraph(fpsData, graphRect, globalMax, style.graphLineColor, currentSampleIndex);
    SDL_Color particleLineColor = {220, 0, 0, 255};
    drawLineGraph(particleData, graphRect, globalMax, particleLineColor, currentSampleIndex);

    SDL_Color titleColor = style.textColor;
    SDL_Texture* titleTex = nullptr;
    initTexture(&titleTex, title.c_str(), titleColor);
    if (titleTex) {
        int tw, th;
        SDL_QueryTexture(titleTex, NULL, NULL, &tw, &th);
        SDL_Rect titleRect = {graphRect.x + (graphRect.w - tw)/2, graphRect.y, tw, th};
        SDL_RenderCopy(renderer, titleTex, NULL, &titleRect);
        SDL_DestroyTexture(titleTex);
    }
}

void GUI::drawLineGraph(const std::vector<float>& data, const SDL_Rect& graphRect, float maxVal, const SDL_Color& lineColor, int currentIndex) {
    int count = (int)data.size();
    if (count < 2) return;

    int leftMargin = 40;
    int bottomMargin = 20;
    float width = (float)(graphRect.w - leftMargin);
    float height = (float)(graphRect.h - bottomMargin);

    SDL_SetRenderDrawColor(renderer, lineColor.r, lineColor.g, lineColor.b, lineColor.a);

    for (int i = 0; i < count - 1; i++) {
        int idx1 = (currentIndex + i) % count;
        int idx2 = (currentIndex + i + 1) % count;

        float val1 = data[idx1];
        float val2 = data[idx2];

        float x1 = graphRect.x + leftMargin + (width * i / (count - 1));
        float y1 = graphRect.y + (height - (val1 / maxVal) * height);
        float x2 = graphRect.x + leftMargin + (width * (i+1) / (count - 1));
        float y2 = graphRect.y + (height - (val2 / maxVal) * height);

        SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
    }
}


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

    std::vector<float> fpsHistory;
    std::vector<float> particleCountHistory;
    size_t maxGraphSamples;
    size_t currentSampleIndex;

    bool pointInRect(int x, int y, const SDL_Rect& rect);
    void renderButton(const SDL_Rect& rect, SDL_Texture* texture, const std::string& label);
    void renderGraph(const SDL_Rect& graphRect, const std::vector<float>& fpsData, const std::vector<float>& particleData, const std::string& title);
    void drawLineGraph(const std::vector<float>& data, const SDL_Rect& graphRect, float maxVal, const SDL_Color& lineColor, int currentIndex);
};

#endif