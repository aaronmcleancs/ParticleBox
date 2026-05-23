#include "gui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
SDL_Color kPanel  { 32, 36, 48, 255 };
SDL_Color kBtn    { 55, 60, 80, 255 };
SDL_Color kBtnHov { 80, 85, 110, 255 };
SDL_Color kBtnAct { 50, 130, 220, 255 };
SDL_Color kText   { 235, 235, 240, 255 };
SDL_Color kDim    { 160, 160, 175, 255 };
SDL_Color kBorder { 60, 70, 90, 255 };

void fillRect(SDL_Renderer *r, const SDL_Rect &rect, SDL_Color c) {
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
  SDL_RenderFillRect(r, &rect);
}
void strokeRect(SDL_Renderer *r, const SDL_Rect &rect, SDL_Color c) {
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
  SDL_RenderDrawRect(r, &rect);
}
} // namespace

GUI::GUI(SDL_Renderer *renderer, TTF_Font *font)
    : renderer_(renderer), font_(font) {
  const int pad = 10;
  const int btnH = 32;
  const int colW = 180;

  int y = 40;
  startBtn_   = { pad,            y, colW, btnH };
  stopBtn_    = { pad + colW + 8, y, colW, btnH };
  y += btnH + 8;
  resetBtn_   = { pad,            y, colW, btnH };
  gravityBtn_ = { pad + colW + 8, y, colW, btnH };
  y += btnH + 8;
  clearBtn_   = { pad,            y, colW, btnH };
  freezeBtn_  = { pad + colW + 8, y, colW, btnH };
  y += btnH + 16;

  mtBtn_      = { pad, y, 368, btnH }; y += btnH + 6;
  gridBtn_    = { pad, y, 368, btnH }; y += btnH + 6;
  modeBtn_    = { pad, y, 368, btnH }; y += btnH + 6;
  helpBtn_    = { pad, y, 368, btnH }; y += btnH + 16;

  inputBox_   = { pad, y, 368, btnH }; y += btnH + 16;

  fpsRect_    = { pad, 8, 200, 22 };
  partRect_   = { 200, 8, 200, 22 };
  modeRect_   = { pad, y, 380, 18 }; y += 30;

  fpsHistory_.resize(maxSamples_, 0.0f);
  countHistory_.resize(maxSamples_, 0.0f);
  velHistory_.resize(maxSamples_, 0.0f);
}

GUI::~GUI() {
  for (auto &kv : textCache_) {
    SDL_DestroyTexture(kv.second);
  }
}

SDL_Texture *GUI::cachedText(const std::string &text, SDL_Color color) {
  // Cache key embeds the colour so the same text in two colours doesn't
  // overwrite each other.
  char ckey[24];
  std::snprintf(ckey, sizeof(ckey), "#%02x%02x%02x|", color.r, color.g, color.b);
  std::string k = std::string(ckey) + text;
  auto it = textCache_.find(k);
  if (it != textCache_.end()) return it->second;

  SDL_Surface *surf = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
  if (!surf) return nullptr;
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, surf);
  SDL_FreeSurface(surf);

  // Keep the cache from growing without bound.
  if (textCache_.size() > 256) {
    auto first = textCache_.begin();
    SDL_DestroyTexture(first->second);
    textCache_.erase(first);
  }
  textCache_.emplace(k, tex);
  return tex;
}

void GUI::drawText(const std::string &text, int x, int y, SDL_Color c) {
  if (text.empty()) return;
  SDL_Texture *t = cachedText(text, c);
  if (!t) return;
  int w, h;
  SDL_QueryTexture(t, nullptr, nullptr, &w, &h);
  SDL_Rect dst{ x, y, w, h };
  SDL_RenderCopy(renderer_, t, nullptr, &dst);
}

void GUI::render(Simulation &sim) {
  updateMetrics(sim);

  SDL_Rect panel{ 0, 0, 400, 800 };
  fillRect(renderer_, panel, kPanel);

  char buf[64];
  std::snprintf(buf, sizeof(buf), "FPS  %5.1f", sim.getFrameRate());
  drawText(buf, fpsRect_.x, fpsRect_.y, {120, 220, 140, 255});
  std::snprintf(buf, sizeof(buf), "Particles  %d", sim.getParticleCount());
  drawText(buf, partRect_.x, partRect_.y, {220, 200, 100, 255});

  renderButton(startBtn_,   sim.isRunning() ? "Running" : "Start", sim.isRunning());
  renderButton(stopBtn_,    "Stop",   !sim.isRunning());
  renderButton(resetBtn_,   "Reset",  false);
  renderButton(gravityBtn_, sim.input().gravityEnabled ? "Gravity ON" : "Gravity OFF",
               sim.input().gravityEnabled);
  renderButton(clearBtn_,   "Clear",  false);
  renderButton(freezeBtn_,  "Freeze", false);

  renderButton(mtBtn_,   sim.isMultithreadingEnabled() ? "Multithread: ON"
                                                       : "Multithread: OFF",
               sim.isMultithreadingEnabled());
  renderButton(gridBtn_, sim.isGridEnabled() ? "Spatial grid: ON"
                                             : "Spatial grid: OFF",
               sim.isGridEnabled());

  std::string modeLabel = std::string("Mouse mode: ") +
                          mouseModeName(sim.input().mode);
  renderButton(modeBtn_, modeLabel, false);
  renderButton(helpBtn_, sim.input().showHelp ? "Help overlay: ON"
                                              : "Help overlay: OFF",
               sim.input().showHelp);

  drawText("Particle count for Reset:", inputBox_.x, inputBox_.y - 20, kDim);
  SDL_Color border = inputActive_ ? SDL_Color{ 110, 170, 255, 255 } : kBorder;
  fillRect(renderer_, inputBox_, kBtn);
  strokeRect(renderer_, inputBox_, border);
  drawText(inputText_, inputBox_.x + 8, inputBox_.y + 7, kText);
  if (inputActive_ && (SDL_GetTicks() / 500) % 2 == 0) {
    int tw = 0;
    SDL_Texture *t = cachedText(inputText_, kText);
    if (t) SDL_QueryTexture(t, nullptr, nullptr, &tw, nullptr);
    SDL_SetRenderDrawColor(renderer_, 230, 230, 240, 255);
    SDL_RenderDrawLine(renderer_,
                       inputBox_.x + 8 + tw, inputBox_.y + 4,
                       inputBox_.x + 8 + tw, inputBox_.y + inputBox_.h - 4);
  }

  // Graphs
  SDL_Rect g1{ 10, 460, 380, 100 };
  drawText("FPS / particles", g1.x, g1.y - 18, kDim);
  renderGraph(g1, fpsHistory_, { 120, 220, 140, 255 }, "fps");
  renderGraph(g1, countHistory_, { 220, 100, 100, 255 }, "n");

  SDL_Rect g2{ 10, 600, 380, 100 };
  drawText("Avg velocity magnitude", g2.x, g2.y - 18, kDim);
  renderGraph(g2, velHistory_, { 100, 170, 255, 255 }, "v");

  drawText("Press H on the sim window for keymap.", 10, 720, kDim);
  drawText("Wheel/PgUp/PgDn change brush/spawn.", 10, 740, kDim);
}

void GUI::updateMetrics(const Simulation &sim) {
  fpsHistory_[sampleIdx_]   = sim.getFrameRate();
  countHistory_[sampleIdx_] = static_cast<float>(sim.getParticleCount());
  Vec2 v = sim.getAverageVelocity();
  velHistory_[sampleIdx_]   = std::sqrt(v.x * v.x + v.y * v.y);
  sampleIdx_ = (sampleIdx_ + 1) % maxSamples_;
}

bool GUI::pointIn(int x, int y, const SDL_Rect &r) {
  return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void GUI::renderButton(const SDL_Rect &rect, const std::string &label,
                       bool toggled) {
  int mx, my;
  SDL_Window *w = SDL_RenderGetWindow(renderer_);
  bool hover = false;
  if (w && (SDL_GetWindowFlags(w) & SDL_WINDOW_MOUSE_FOCUS)) {
    SDL_GetMouseState(&mx, &my);
    hover = pointIn(mx, my, rect);
  }
  SDL_Color bg = toggled ? kBtnAct : (hover ? kBtnHov : kBtn);
  fillRect(renderer_, rect, bg);
  strokeRect(renderer_, rect, kBorder);

  SDL_Texture *t = cachedText(label, kText);
  if (t) {
    int tw, th;
    SDL_QueryTexture(t, nullptr, nullptr, &tw, &th);
    SDL_Rect dst{ rect.x + (rect.w - tw) / 2,
                  rect.y + (rect.h - th) / 2,
                  tw, th };
    SDL_RenderCopy(renderer_, t, nullptr, &dst);
  }
}

void GUI::renderGraph(const SDL_Rect &rect, const std::vector<float> &data,
                      SDL_Color color, const std::string & /*title*/,
                      float maxOverride) {
  fillRect(renderer_, rect, { 10, 14, 22, 255 });
  strokeRect(renderer_, rect, kBorder);

  float maxV = maxOverride;
  if (maxV <= 0.0f) {
    for (float v : data) maxV = std::max(maxV, v);
    if (maxV < 1.0f) maxV = 1.0f;
  }

  SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
  const std::size_t n = data.size();
  for (std::size_t i = 0; i + 1 < n; ++i) {
    std::size_t i1 = (sampleIdx_ + i) % n;
    std::size_t i2 = (sampleIdx_ + i + 1) % n;
    float x1 = rect.x + static_cast<float>(i) / (n - 1) * rect.w;
    float x2 = rect.x + static_cast<float>(i + 1) / (n - 1) * rect.w;
    float y1 = rect.y + rect.h - (data[i1] / maxV) * rect.h;
    float y2 = rect.y + rect.h - (data[i2] / maxV) * rect.h;
    SDL_RenderDrawLine(renderer_, (int)x1, (int)y1, (int)x2, (int)y2);
  }
}

void GUI::handleEvent(const SDL_Event &event, Simulation &sim) {
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    int x = event.button.x, y = event.button.y;
    if (pointIn(x, y, startBtn_))   sim.start();
    else if (pointIn(x, y, stopBtn_))  sim.stop();
    else if (pointIn(x, y, resetBtn_)) {
      try { sim.reset(std::stoi(inputText_.empty() ? "0" : inputText_)); }
      catch (...) { sim.reset(0); }
    }
    else if (pointIn(x, y, gravityBtn_)) sim.toggleGravity();
    else if (pointIn(x, y, mtBtn_))      sim.toggleMultithreading();
    else if (pointIn(x, y, gridBtn_))    sim.toggleGrid();
    else if (pointIn(x, y, clearBtn_))   sim.clearParticles();
    else if (pointIn(x, y, freezeBtn_))  sim.freezeAll();
    else if (pointIn(x, y, modeBtn_)) {
      // Cycle mouse mode
      int v = (static_cast<int>(sim.input().mode) + 1) %
              static_cast<int>(MouseMode::Count);
      sim.input().mode = static_cast<MouseMode>(v);
    }
    else if (pointIn(x, y, helpBtn_))    sim.input().showHelp = !sim.input().showHelp;

    inputActive_ = pointIn(x, y, inputBox_);
  } else if (event.type == SDL_TEXTINPUT && inputActive_) {
    inputText_ += event.text.text;
  } else if (event.type == SDL_KEYDOWN && inputActive_) {
    if (event.key.keysym.sym == SDLK_BACKSPACE && !inputText_.empty()) {
      inputText_.pop_back();
    } else if (event.key.keysym.sym == SDLK_RETURN) {
      try { sim.reset(std::stoi(inputText_.empty() ? "0" : inputText_)); }
      catch (...) {}
      inputActive_ = false;
    }
  }
}
