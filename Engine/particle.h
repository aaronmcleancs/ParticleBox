#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL.h>
#include <cmath> 

#ifdef __APPLE__
#include <simd/simd.h>
#endif

struct Vec2 {
    float x, y;

    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

#ifdef __APPLE__
    // SIMD optimized operations for Apple Silicon
    Vec2 operator+(const Vec2& other) const {
        simd_float2 a = {x, y};
        simd_float2 b = {other.x, other.y};
        simd_float2 result = a + b;
        return {result.x, result.y};
    }

    Vec2 operator-(const Vec2& other) const {
        simd_float2 a = {x, y};
        simd_float2 b = {other.x, other.y};
        simd_float2 result = a - b;
        return {result.x, result.y};
    }

    Vec2 operator*(float scalar) const {
        simd_float2 a = {x, y};
        simd_float2 result = a * scalar;
        return {result.x, result.y};
    }
    
    Vec2 operator/(float scalar) const {
        if (scalar == 0.0f) return {0.0f, 0.0f};
        float invScalar = 1.0f / scalar;
        return *this * invScalar; // Multiply by inverse is faster than division
    }
#else
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
#endif

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

    // Fast inverse square root (Quake III optimization)
    static inline float fastInvSqrt(float number) {
        union {
            float f;
            uint32_t i;
        } conv;
        
        float x2 = number * 0.5f;
        conv.f = number;
        conv.i = 0x5f3759df - (conv.i >> 1);
        conv.f = conv.f * (1.5f - (x2 * conv.f * conv.f));
        return conv.f;
    }

    float magnitudeSq() const {
        return x * x + y * y;
    }

    float magnitude() const {
        float magSq = magnitudeSq();
        return std::sqrt(magSq);
    }

    Vec2 norm() const {
        float magSq = magnitudeSq();
        if (magSq < 0.0001f) return Vec2();
        
        float invMag = fastInvSqrt(magSq);
        return *this * invMag;
    }

    float dot(const Vec2& other) const {
#ifdef __APPLE__
        simd_float2 a = {x, y};
        simd_float2 b = {other.x, other.y};
        return simd_dot(a, b);
#else
        return x * other.x + y * other.y;
#endif
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