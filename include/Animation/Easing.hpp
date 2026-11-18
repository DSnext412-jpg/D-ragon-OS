

#pragma once

#include <Animation/AnimationCurve.hpp>

#include <cmath>

namespace DragonOS::Animation {

class Easing final {
public:
    Easing() = delete;

    static constexpr float Linear(float t) noexcept { return t; }

    static constexpr float EaseIn(float t) noexcept { return t * t; }

    static constexpr float EaseOut(float t) noexcept { return 1.0f - (1.0f - t) * (1.0f - t); }

    static constexpr float EaseInOut(float t) noexcept
    {
        return t < 0.5f
            ? 2.0f * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    static constexpr float Cubic(float t) noexcept
    {
        return t < 0.5f
            ? 4.0f * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    static constexpr float Quartic(float t) noexcept
    {
        return t < 0.5f
            ? 8.0f * t * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    static constexpr float Quintic(float t) noexcept
    {
        return t < 0.5f
            ? 16.0f * t * t * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }


    static float Bounce(float t) noexcept;

    static float Elastic(float t) noexcept;

    static float Back(float t) noexcept;

    static float Apply(AnimationCurve curve, float t) noexcept;
};

} 
