#include "particle.h"
#include <algorithm>

void Particle::update(const Vec2& force, float deltaTime) {
    Vec2 acceleration = force / mass; // Using division by scalar for acceleration
    velocity += acceleration * deltaTime; // Update velocity with time step
    position += velocity * deltaTime; // Update position with new velocity
}

// Implementation of Particle::render
void Particle::render(SDL_Renderer* renderer) {
    float speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Color orange = {255, 165, 0, 255};
    float maxSpeed = 50.0;  // Adjust based on expected speed range
    float normSpeed = std::min(speed / maxSpeed, 1.0f);

    SDL_Color color;
    color.r = (Uint8)(blue.r + normSpeed * (orange.r - blue.r));
    color.g = (Uint8)(blue.g + normSpeed * (orange.g - blue.g));
    color.b = (Uint8)(blue.b + normSpeed * (orange.b - blue.b));
    color.a = 255;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    drawCircle(renderer, int(position.x), int(position.y), int(radius));
}


void Particle::drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;  // Horizontal offset
            int dy = radius - h;  // Vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}
