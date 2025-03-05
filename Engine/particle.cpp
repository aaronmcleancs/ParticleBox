
#include "particle.h"
#include <SDL.h>
#include <SDL_render.h>
#include <cmath>
#include <unordered_map>


SDL_Texture* createCircleTexture(SDL_Renderer* renderer, int radius) {
    int diameter = radius * 2;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, diameter, diameter);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    
    
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = -radius; y <= radius; ++y) {
        int dx = static_cast<int>(std::sqrt(radius * radius - y * y));
        SDL_RenderDrawLine(renderer, radius - dx, radius + y, radius + dx, radius + y);
    }
    
    SDL_SetRenderTarget(renderer, nullptr);
    return texture;
}


static std::unordered_map<int, SDL_Texture*> circleTextureCache;

void Particle::update(const Vec2& force, float deltaTime) {
    Vec2 acceleration = force * invMass;
    velocity += acceleration * deltaTime; 
    position += velocity * deltaTime;
}

void Particle::render(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        
        return;
    }
    
    
    if (std::isnan(position.x) || std::isnan(position.y) || 
        std::isnan(velocity.x) || std::isnan(velocity.y)) {
        return;
    }

    
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    SDL_Color blue = {40, 40, 255, 255};
    SDL_Color orange = {200, 20, 20, 255};
    float maxSpeed = 50.0f;
    float normSpeed = std::min(speed / maxSpeed, 1.0f);
    
    SDL_Color finalColor;
    finalColor.r = static_cast<Uint8>(blue.r + normSpeed * (orange.r - blue.r));
    finalColor.g = static_cast<Uint8>(blue.g + normSpeed * (orange.g - blue.g));
    finalColor.b = static_cast<Uint8>(blue.b + normSpeed * (orange.b - blue.b));
    finalColor.a = 255;

    int rad = static_cast<int>(radius);
    if (circleTextureCache.find(rad) == circleTextureCache.end()) {
        circleTextureCache[rad] = createCircleTexture(renderer, rad);
    }
    SDL_Texture* circleTexture = circleTextureCache[rad];

    
    SDL_SetTextureColorMod(circleTexture, finalColor.r, finalColor.g, finalColor.b);
    
    SDL_Rect dstRect = {
        static_cast<int>(position.x - radius),
        static_cast<int>(position.y - radius),
        rad * 2,
        rad * 2
    };
    SDL_RenderCopy(renderer, circleTexture, nullptr, &dstRect);
}