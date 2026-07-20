/**
 * @file    Easing.cpp
 * @brief   Runtime easing functions (complex curves using std::cmath).
 */

#include <Animation/Easing.hpp>

#include <cmath>

namespace DragonOS::Animation {

float Easing::Bounce(float t) noexcept
{
    // Standard bounce easing.
    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;

    if (t < 1.0f / d1)
    {
        return n1 * t * t;
    }
    else if (t < 2.0f / d1)
    {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    }
    else if (t < 2.5f / d1)
    {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    }
    else
    {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

float Easing::Elastic(float t) noexcept
{
    if (t == 0.0f || t == 1.0f) { return t; }

    constexpr float c4 = (2.0f * 3.14159265f) / 3.0f;
    return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
}

float Easing::Back(float t) noexcept
{
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

float Easing::Apply(AnimationCurve curve, float t) noexcept
{
    switch (curve)
    {
    case AnimationCurve::Linear:    return Linear(t);
    case AnimationCurve::EaseIn:    return EaseIn(t);
    case AnimationCurve::EaseOut:   return EaseOut(t);
    case AnimationCurve::EaseInOut: return EaseInOut(t);
    case AnimationCurve::Cubic:     return Cubic(t);
    case AnimationCurve::Quartic:   return Quartic(t);
    case AnimationCurve::Quintic:   return Quintic(t);
    case AnimationCurve::Bounce:    return Bounce(t);
    case AnimationCurve::Elastic:   return Elastic(t);
    case AnimationCurve::Back:      return Back(t);
    default:                        return t;
    }
}

} // namespace DragonOS::Animation
