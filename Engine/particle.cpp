#include "particle.h"
#include <iostream>
#include <algorithm>

void Particle::update(const Vec2& force, float deltaTime) {
    Vec2 acceleration = force / mass;
    velocity += acceleration * deltaTime; 
    position += velocity * deltaTime;
}

void Particle::render(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        std::cerr << "Error: Renderer is null!" << std::endl;
        return;
    }

    if (std::isnan(position.x) || std::isnan(position.y) || std::isnan(velocity.x) || std::isnan(velocity.y)) {
        std::cerr << "Error: Invalid particle position or velocity!" << std::endl;
        return;
    }

    float speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    SDL_Color blue = {40, 40, 255, 255};
    SDL_Color orange = {255, 0, 90, 255};
    float maxSpeed = 50.0f; 
    float normSpeed = std::min(speed / maxSpeed, 1.0f);

    SDL_Color color;
    color.r = static_cast<Uint8>(blue.r + normSpeed * (orange.r - blue.r));
    color.g = static_cast<Uint8>(blue.g + normSpeed * (orange.g - blue.g));
    color.b = static_cast<Uint8>(blue.b + normSpeed * (orange.b - blue.b));
    color.a = 255;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    drawCircle(renderer, static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(radius));
}

void Particle::drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;  
            int dy = radius - h;  
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}
