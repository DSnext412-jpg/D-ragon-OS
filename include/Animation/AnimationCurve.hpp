/**
 * @file    AnimationCurve.hpp
 * @brief   Enumeration of built-in animation easing curves.
 */

#pragma once

namespace DragonOS::Animation {

/**
 * @brief  Identifies which easing function an Animation uses.
 *
 * The curve determines how the raw time progress [0, 1] maps to
 * an eased value before being passed to the animation callback.
 */
enum class AnimationCurve {
    Linear,    ///< Constant velocity.
    EaseIn,    ///< Slow start, fast end (quadratic).
    EaseOut,   ///< Fast start, slow end (quadratic).
    EaseInOut, ///< Slow start and end, fast middle (quadratic).
    Cubic,     ///< Cubic in/out.
    Quartic,   ///< Quartic in/out.
    Quintic,   ///< Quintic in/out.
    Bounce,    ///< Simulated bounce at the end.
    Elastic,   ///< Overshoot with oscillation.
    Back,      ///< Overshoot then settle.
};

} // namespace DragonOS::Animation
