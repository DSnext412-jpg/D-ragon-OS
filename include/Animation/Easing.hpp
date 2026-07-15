/**
 * @file    Easing.hpp
 * @brief   Stateless easing functions for animation curves.
 *
 * Every function maps a normalised time  t ∈ [0, 1]  to an eased
 * output value (also typically in [0, 1]).
 */

#pragma once

#include <Animation/AnimationCurve.hpp>

#include <cmath>

namespace DragonOS::Animation {

/**
 * @brief  Pure functions that transform linear progress into eased motion.
 *
 * Usage:
 * @code
 *   float raw   = 0.45f;                  // linear progress
 *   float eased = Easing::EaseInOut(raw); // eased value
 * @endcode
 */
class Easing final {
public:
    Easing() = delete;

    // ── Simple polynomial curves (constexpr) ─────────────────────────────

    /// @brief  Linear — identity.
    static constexpr float Linear(float t) noexcept { return t; }

    /// @brief  Quadratic ease-in.
    static constexpr float EaseIn(float t) noexcept { return t * t; }

    /// @brief  Quadratic ease-out.
    static constexpr float EaseOut(float t) noexcept { return 1.0f - (1.0f - t) * (1.0f - t); }

    /// @brief  Quadratic ease-in-out.
    static constexpr float EaseInOut(float t) noexcept
    {
        return t < 0.5f
            ? 2.0f * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    /// @brief  Cubic ease-in-out.
    static constexpr float Cubic(float t) noexcept
    {
        return t < 0.5f
            ? 4.0f * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    /// @brief  Quartic ease-in-out.
    static constexpr float Quartic(float t) noexcept
    {
        return t < 0.5f
            ? 8.0f * t * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    /// @brief  Quintic ease-in-out.
    static constexpr float Quintic(float t) noexcept
    {
        return t < 0.5f
            ? 16.0f * t * t * t * t * t
            : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
    }

    // ── Complex curves (runtime, require std::sin / std::pow) ────────────

    /// @brief  Simulated bounce at the end of the animation.
    static float Bounce(float t) noexcept;

    /// @brief  Overshoot with decaying oscillation.
    static float Elastic(float t) noexcept;

    /// @brief  Overshoot past the target then settle back.
    static float Back(float t) noexcept;

    // ── Dispatcher ──────────────────────────────────────────────────────

    /**
     * @brief  Apply the given curve to a normalised time value.
     *
     * @param curve  The easing curve to use.
     * @param t      Raw progress in [0, 1].
     *
     * @return The eased value.
     */
    static float Apply(AnimationCurve curve, float t) noexcept;
};

} // namespace DragonOS::Animation
