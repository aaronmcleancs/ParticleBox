#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL.h>
#include <cmath> // For sqrt, used in Vec2::magnitude

struct Vec2 {
    float x, y;

    // Default constructor with optional parameters
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

    // Vector addition
    Vec2 operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }

    // Vector subtraction
    Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }

    // Scalar multiplication
    Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    // Scalar division
    Vec2 operator/(float scalar) const {
        return {x / scalar, y / scalar};  // Note: No division by zero handling
    }

    // Compound assignment (vector addition)
    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Compound assignment (vector subtraction)
    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Calculate magnitude (length) of the vector
    float magnitude() const {
        return sqrt(x * x + y * y);
    }

    // Normalize the vector
    Vec2 norm() const {
        float mag = magnitude();
        return mag > 0 ? *this * (1 / mag) : Vec2();  // Avoid division by zero
    }

    // Dot product of two vectors
    float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }
};

class Particle {
public:
    Vec2 position, velocity;
    SDL_Color color;
    float radius, mass, dipoleMoment, exclusionConstant;
    int type; // For categorizing particle types

    // Ensure this constructor is exactly as follows:
    Particle(Vec2 pos, Vec2 vel, SDL_Color col, float r, float m, float dipole, float exclusion, int t)
        : position(pos), velocity(vel), color(col), radius(r), mass(m), dipoleMoment(dipole), exclusionConstant(exclusion), type(t) {}
    
    void update(const Vec2& force, float deltaTime);
    void render(SDL_Renderer* renderer);
};


#endif // PARTICLE_H