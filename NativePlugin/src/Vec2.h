#pragma once

#include <cmath>

namespace angrybirds {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& other) const noexcept {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const noexcept {
        return {x - other.x, y - other.y};
    }

    Vec2 operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    Vec2 operator/(float scalar) const noexcept {
        return {x / scalar, y / scalar};
    }

    Vec2& operator+=(const Vec2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

inline float Dot(const Vec2& left, const Vec2& right) noexcept {
    return left.x * right.x + left.y * right.y;
}

inline float LengthSquared(const Vec2& value) noexcept {
    return Dot(value, value);
}

inline bool IsFinite(const Vec2& value) noexcept {
    return std::isfinite(value.x) && std::isfinite(value.y);
}

}
