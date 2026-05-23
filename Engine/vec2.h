#ifndef VEC2_H
#define VEC2_H

#include <cmath>

struct Vec2 {
  float x, y;
  constexpr Vec2(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}

  constexpr Vec2 operator+(const Vec2 &o) const { return {x + o.x, y + o.y}; }
  constexpr Vec2 operator-(const Vec2 &o) const { return {x - o.x, y - o.y}; }
  constexpr Vec2 operator*(float s)        const { return {x * s, y * s}; }
  constexpr Vec2 operator/(float s)        const { return {x / s, y / s}; }
  constexpr Vec2 operator-()               const { return {-x, -y}; }

  Vec2 &operator+=(const Vec2 &o) { x += o.x; y += o.y; return *this; }
  Vec2 &operator-=(const Vec2 &o) { x -= o.x; y -= o.y; return *this; }
  Vec2 &operator*=(float s)       { x *= s;   y *= s;   return *this; }

  float magnitude()    const { return std::sqrt(x * x + y * y); }
  float magnitudeSq()  const { return x * x + y * y; }

  Vec2 normalized() const {
    float m = magnitude();
    return m > 1e-6f ? Vec2{x / m, y / m} : Vec2{0.0f, 0.0f};
  }

  static float dot(const Vec2 &a, const Vec2 &b) {
    return a.x * b.x + a.y * b.y;
  }
};

#endif
