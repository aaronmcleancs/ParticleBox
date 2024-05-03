#include "particle.h"

void Particle::update(const Vec2& force, float deltaTime) {
    Vec2 acceleration = force / mass; // Using division by scalar for acceleration
    velocity += acceleration * deltaTime; // Update velocity with time step
    position += velocity * deltaTime; // Update position with new velocity
}

void Particle::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); // Set the particle's color
    SDL_Rect rect = {int(position.x - radius), int(position.y - radius), int(radius * 2), int(radius * 2)}; // Define the rectangle for the particle
    SDL_RenderFillRect(renderer, &rect); // Render the particle as a filled rectangle (circle approximation)
}
