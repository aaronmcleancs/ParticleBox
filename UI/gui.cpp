#include "gui.h"
#include "simulation.h"
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <string>

struct GUIStyle {
  SDL_Color backgroundColor = {20, 20, 30,
                               200}; // Dark blue-ish, semi-transparent
  SDL_Color panelColor = {30, 30, 40, 220};
  SDL_Color buttonColor = {50, 50, 70, 255};
  SDL_Color buttonHoverColor = {70, 70, 90, 255};
  SDL_Color buttonActiveColor = {60, 100, 200, 255}; // Accent color
  SDL_Color textColor = {240, 240, 240, 255};
  SDL_Color inputActiveColor = {40, 40, 50, 255};
  SDL_Color borderColor = {80, 80, 100, 255};

  SDL_Color graphBackground = {0, 0, 0, 150};
  SDL_Color graphLineColor = {0, 200, 100, 255}; // Green accent
  SDL_Color graphAxisColor = {120, 120, 120, 255};
};

static void drawRect(SDL_Renderer *renderer, const SDL_Rect &rect,
                     const SDL_Color &color, bool filled = true) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  if (filled)
    SDL_RenderFillRect(renderer, &rect);
  else
    SDL_RenderDrawRect(renderer, &rect);
}

GUI::GUI(SDL_Renderer *renderer, TTF_Font *font)
    : renderer(renderer), font(font), inputActive(false),
      particleCountInput("100") {

  // HUD Layout
  int padding = 10;
  int btnH = 35;
  int btnW = 120;
  int startY = 50;

  // Top Left Controls
  startButton = {padding, startY, btnW, btnH};
  stopButton = {padding + btnW + padding, startY, btnW, btnH};
  resetButton = {padding, startY + btnH + padding, btnW, btnH};
  gravityButton = {padding + btnW + padding, startY + btnH + padding, btnW,
                   btnH};

  // Toggles
  int toggleY = startY + (btnH + padding) * 2 + 10;
  multithreadingButton = {padding, toggleY, 250, btnH};
  gridButton = {padding, toggleY + btnH + padding, 250, btnH};
  reducedPairwiseButton = {padding, toggleY + (btnH + padding) * 2, 250, btnH};

  // Stats (Top Right)
  particleCountRect = {10, 10, 200, 30};
  frameRateRect = {220, 10, 150, 30};

  // Input
  particleCountInputRect = {padding, toggleY + (btnH + padding) * 3 + 10, 250,
                            btnH};

  startTexture = nullptr;
  stopTexture = nullptr;
  resetTexture = nullptr;
  gravityTexture = nullptr;
  multithreadingTexture = nullptr;
  gridTexture = nullptr;
  reducedPairwiseTexture = nullptr;
  particleCountTexture = nullptr;
  frameRateTexture = nullptr;
  particleCountInputTexture = nullptr;

  GUIStyle style;
  initTexture(&startTexture, "Start", style.textColor);
  initTexture(&stopTexture, "Stop", style.textColor);
  initTexture(&resetTexture, "Reset", style.textColor);
  initTexture(&gravityTexture, "Toggle Gravity", style.textColor);
  initTexture(&multithreadingTexture, "Toggle Multithreading", style.textColor);
  initTexture(&gridTexture, "Toggle Grid", style.textColor);
  initTexture(&reducedPairwiseTexture, "Toggle Pairwise", style.textColor);

  maxGraphSamples = 60;
  fpsHistory.resize(maxGraphSamples, 0.0f);
  particleCountHistory.resize(maxGraphSamples, 0.0f);
  averageVelocityHistory.resize(maxGraphSamples, 0.0f);
  currentSampleIndex = 0;
}

GUI::~GUI() {
  SDL_DestroyTexture(startTexture);
  SDL_DestroyTexture(stopTexture);
  SDL_DestroyTexture(resetTexture);
  SDL_DestroyTexture(gravityTexture);
  SDL_DestroyTexture(multithreadingTexture);
  SDL_DestroyTexture(gridTexture);
  SDL_DestroyTexture(reducedPairwiseTexture);
  if (particleCountTexture)
    SDL_DestroyTexture(particleCountTexture);
  if (frameRateTexture)
    SDL_DestroyTexture(frameRateTexture);
  if (particleCountInputTexture)
    SDL_DestroyTexture(particleCountInputTexture);
}

void GUI::render(Simulation &simulation) {
  updateMetricsDisplay(simulation);

  GUIStyle style;

  SDL_Rect panelRect = {0, 0, 400, 800};
  drawRect(renderer, panelRect, style.panelColor);

  renderButton(startButton, startTexture, "Start");
  renderButton(stopButton, stopTexture, "Stop");
  renderButton(resetButton, resetTexture, "Reset");
  renderButton(gravityButton, gravityTexture, "Toggle Gravity");

  std::string mtLabel = simulation.isMultithreadingEnabled()
                            ? "Multithreading: ON"
                            : "Multithreading: OFF";
  initTexture(&multithreadingTexture, mtLabel.c_str(), style.textColor);
  renderButton(multithreadingButton, multithreadingTexture, mtLabel);

  std::string gridLabel = simulation.isGridEnabled() ? "Grid: ON" : "Grid: OFF";
  initTexture(&gridTexture, gridLabel.c_str(), style.textColor);
  renderButton(gridButton, gridTexture, gridLabel);

  std::string pairwiseLabel = simulation.isReducedPairwiseComparisonsEnabled()
                                  ? "Pairwise: ON"
                                  : "Pairwise: OFF";
  initTexture(&reducedPairwiseTexture, pairwiseLabel.c_str(), style.textColor);
  renderButton(reducedPairwiseButton, reducedPairwiseTexture, pairwiseLabel);

  if (particleCountTexture) {
    SDL_RenderCopy(renderer, particleCountTexture, NULL, &particleCountRect);
  }
  if (frameRateTexture) {
    SDL_Rect fpsBg = frameRateRect;
    fpsBg.x -= 5;
    fpsBg.y -= 5;
    fpsBg.w += 10;
    fpsBg.h += 10;
    drawRect(renderer, fpsBg, {50, 50, 50, 255});
    SDL_RenderCopy(renderer, frameRateTexture, NULL, &frameRateRect);
  }

  // Input Field
  // Label
  SDL_Color labelColor = {180, 180, 180, 255};
  SDL_Texture *labelTex = nullptr;
  initTexture(&labelTex, "Particle Count:", labelColor);
  SDL_Rect labelRect = {particleCountInputRect.x, particleCountInputRect.y - 25,
                        120, 20};
  SDL_RenderCopy(renderer, labelTex, NULL, &labelRect);
  SDL_DestroyTexture(labelTex);

  // Input Box
  SDL_Color inputBg = {40, 40, 50, 255};
  SDL_Color inputBorder = {80, 80, 100, 255};
  if (inputActive) {
    inputBg = {50, 50, 60, 255};
    inputBorder = {100, 150, 255, 255}; // Active highlight
  }

  drawRect(renderer, particleCountInputRect, inputBg, true); // Fill

  // Border
  SDL_SetRenderDrawColor(renderer, inputBorder.r, inputBorder.g, inputBorder.b,
                         inputBorder.a);
  SDL_RenderDrawRect(renderer, &particleCountInputRect);

  // Text
  initTexture(&particleCountInputTexture, particleCountInput.c_str(),
              style.textColor);
  if (particleCountInputTexture) {
    int textW, textH;
    SDL_QueryTexture(particleCountInputTexture, NULL, NULL, &textW, &textH);

    // Center vertically, left align with padding
    SDL_Rect textRect = {particleCountInputRect.x + 10,
                         particleCountInputRect.y +
                             (particleCountInputRect.h - textH) / 2,
                         textW, textH};
    // Clip if too long
    if (textRect.w > particleCountInputRect.w - 20) {
      textRect.w = particleCountInputRect.w - 20;
    }
    SDL_RenderCopy(renderer, particleCountInputTexture, NULL, &textRect);
  }

  // Cursor (if active)
  if (inputActive && (SDL_GetTicks() / 500) % 2 == 0) {
    int textW = 0;
    if (particleCountInputTexture) {
      SDL_QueryTexture(particleCountInputTexture, NULL, NULL, &textW, NULL);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(
        renderer, particleCountInputRect.x + 10 + textW,
        particleCountInputRect.y + 10, particleCountInputRect.x + 10 + textW,
        particleCountInputRect.y + particleCountInputRect.h - 10);
  }

  SDL_Rect graphRect = {10, 360, 380, 150};
  renderGraph(graphRect, fpsHistory, particleCountHistory, "");

  SDL_Rect avgVelGraphRect = {10, 530, 380, 150};
  renderAverageVelocityGraph(avgVelGraphRect, averageVelocityHistory, "");
}

void GUI::updateMetricsDisplay(const Simulation &simulation) {
  fpsHistory[currentSampleIndex] = simulation.getFrameRate();
  particleCountHistory[currentSampleIndex] =
      (float)simulation.getParticleCount();

  Vec2 avgVel = simulation.getAverageVelocity();
  float avgVelMagnitude = std::sqrt(avgVel.x * avgVel.x + avgVel.y * avgVel.y);
  averageVelocityHistory[currentSampleIndex] = avgVelMagnitude;

  currentSampleIndex = (currentSampleIndex + 1) % maxGraphSamples;

  GUIStyle style;
  std::string particleCountText =
      "Particles: " + std::to_string(simulation.getParticleCount());
  std::string frameRateText =
      "FPS: " + std::to_string((int)std::round(simulation.getFrameRate()));

  initTexture(&particleCountTexture, particleCountText.c_str(),
              style.textColor);
  initTexture(&frameRateTexture, frameRateText.c_str(), {0, 255, 0, 255});
}

void GUI::initTexture(SDL_Texture **texture, const char *text,
                      SDL_Color color) {
  if (*texture) {
    SDL_DestroyTexture(*texture);
    *texture = nullptr;
  }
  SDL_Surface *surface = TTF_RenderText_Blended(font, text, color);
  *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
}

void GUI::handleEvent(SDL_Event &event, Simulation &simulation) {
  if (event.type == SDL_MOUSEBUTTONDOWN) {

    int x = event.button.x;
    int y = event.button.y;

    if (pointInRect(x, y, startButton)) {
      simulation.start();
    } else if (pointInRect(x, y, stopButton)) {
      simulation.stop();
    } else if (pointInRect(x, y, resetButton)) {
      int newParticleCountReset =
          std::stoi(particleCountInput.empty() ? "0" : particleCountInput);
      simulation.reset(newParticleCountReset);
    } else if (pointInRect(x, y, gravityButton)) {
      simulation.toggleGravity();
    } else if (pointInRect(x, y, multithreadingButton)) {
      simulation.toggleMultithreading();
    } else if (pointInRect(x, y, gridButton)) {
      simulation.toggleGrid();
    } else if (pointInRect(x, y, reducedPairwiseButton)) {
      simulation.toggleReducedPairwiseComparisons();
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
        int newParticleCount =
            std::stoi(particleCountInput.empty() ? "0" : particleCountInput);
        simulation.setParticle(newParticleCount);
      } catch (const std::invalid_argument &) {
        particleCountInput = "0";
      }
      inputActive = false;
    }
  }
}

bool GUI::pointInRect(int x, int y, const SDL_Rect &rect) {
  return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y &&
          y <= rect.y + rect.h);
}

void GUI::renderButton(const SDL_Rect &rect, SDL_Texture *texture,
                       const std::string &label, bool toggled) {
  GUIStyle style;

  int mouseX, mouseY;
  Uint32 mouseState;

  SDL_Window *window = SDL_RenderGetWindow(renderer);
  if (window) {

    bool isMouseFocused = (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS);

    if (isMouseFocused) {
      mouseState = SDL_GetMouseState(&mouseX, &mouseY);
      bool hovered = pointInRect(mouseX, mouseY, rect);

      SDL_Color bgColor = hovered ? style.buttonHoverColor : style.buttonColor;
      if (toggled) {
        bgColor = {100, 180, 100, 255};
      }
      drawRect(renderer, rect, bgColor);
    } else {

      SDL_Color bgColor = style.buttonColor;
      if (toggled) {
        bgColor = {100, 180, 100, 255};
      }
      drawRect(renderer, rect, bgColor);
    }
  } else {

    SDL_Color bgColor = style.buttonColor;
    if (toggled) {
      bgColor = {100, 180, 100, 255};
    }
    drawRect(renderer, rect, bgColor);
  }

  if (texture) {
    SDL_RenderCopy(renderer, texture, NULL, &rect);
  } else {
    SDL_Texture *tempTex = nullptr;
    initTexture(&tempTex, label.c_str(), style.textColor);
    SDL_RenderCopy(renderer, tempTex, NULL, &rect);
    SDL_DestroyTexture(tempTex);
  }
}

void GUI::renderGraph(const SDL_Rect &graphRect,
                      const std::vector<float> &fpsData,
                      const std::vector<float> &particleData,
                      const std::string &title) {
  GUIStyle style;
  drawRect(renderer, graphRect, style.graphBackground);

  float maxFPS = *std::max_element(fpsData.begin(), fpsData.end());
  float maxParticles =
      *std::max_element(particleData.begin(), particleData.end());
  float globalMax = std::max(maxFPS, maxParticles);
  if (globalMax < 1.0f)
    globalMax = 1.0f;

  drawLineGraph(fpsData, graphRect, globalMax, style.graphLineColor,
                currentSampleIndex);

  SDL_Color particleLineColor = {220, 0, 0, 255};
  drawLineGraph(particleData, graphRect, globalMax, particleLineColor,
                currentSampleIndex);
}

void GUI::renderAverageVelocityGraph(const SDL_Rect &graphRect,
                                     const std::vector<float> &avgVelData,
                                     const std::string &title) {
  GUIStyle style;
  drawRect(renderer, graphRect, style.graphBackground);

  float maxVel = *std::max_element(avgVelData.begin(), avgVelData.end());
  if (maxVel < 1.0f)
    maxVel = 1.0f;

  SDL_Color velLineColor = {0, 0, 220, 255};
  drawLineGraph(avgVelData, graphRect, maxVel, velLineColor,
                currentSampleIndex);
}

void GUI::drawLineGraph(const std::vector<float> &data,
                        const SDL_Rect &graphRect, float maxVal,
                        const SDL_Color &lineColor, int currentIndex) {
  int count = (int)data.size();
  if (count < 2)
    return;

  float width = (float)graphRect.w;
  float height = (float)graphRect.h;

  SDL_SetRenderDrawColor(renderer, lineColor.r, lineColor.g, lineColor.b,
                         lineColor.a);

  for (int i = 0; i < count - 1; i++) {
    int idx1 = (currentSampleIndex + i) % count;
    int idx2 = (currentSampleIndex + i + 1) % count;

    float val1 = data[idx1];
    float val2 = data[idx2];

    float x1 = graphRect.x + (width * i / (count - 1));
    float y1 = graphRect.y + (height - (val1 / maxVal) * height);
    float x2 = graphRect.x + (width * (i + 1) / (count - 1));
    float y2 = graphRect.y + (height - (val2 / maxVal) * height);

    SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
  }
}