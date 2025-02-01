#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL.h>
#include <cmath> 

struct Vec2 {
    float x, y;

    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }
    
    Vec2 operator/(float scalar) const {
        return {x / scalar, y / scalar};
    }

    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    float magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 norm() const {
        float mag = magnitude();
        return mag > 0 ? *this * (1 / mag) : Vec2(); 
    }

    float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }
};

class Particle {
public:
    Vec2 position, velocity;
    SDL_Color color;
    float radius, mass, invMass, dipoleMoment, exclusionConstant, repulsionFactor;
    int type;
    Particle(Vec2 pos, Vec2 vel, SDL_Color col, float r, float m, float dipole, float exclusion, float repulsion, int t)
        : position(pos), velocity(vel), color(col), radius(r), mass(m), dipoleMoment(dipole), exclusionConstant(exclusion), repulsionFactor(repulsion), type(t)
    {
         invMass = (mass != 0.0f) ? 1.0f / mass : 0.0f;
    }
    void update(const Vec2& force, float deltaTime);
    void render(SDL_Renderer* renderer);
private:
    static void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);
};

#endif