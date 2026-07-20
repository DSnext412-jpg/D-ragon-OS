/**
 * @file    Interpolator.hpp
 * @brief   Generic template-based interpolation between two values.
 *
 * Supports float, double, and int out of the box.  User-defined types
 * can opt in by specialising Interpolator or providing an
 * `operator+(T)`, `operator-(T)`, and `operator*(float)`.
 */

#pragma once

#include <concepts>
#include <type_traits>

namespace DragonOS::Animation {

/**
 * @brief  Compile-time concept for arithmetic types usable by Interpolator.
 */
template<typename T>
concept Interpolatable = std::is_arithmetic_v<T>;

/**
 * @brief  Linear interpolation between two values.
 *
 * @tparam T  Value type (must satisfy Interpolatable or be specialised).
 *
 * Performs  `from + (to - from) * t`  for floating-point types and
 * a rounded equivalent for integer types.
 */
template<typename T>
class Interpolator final {
public:
    Interpolator() = delete;

    /**
     * @brief  Linearly interpolate between  @p from  and  @p to.
     *
     * @param from  Start value.
     * @param to    End value.
     * @param t     Normalised progress in [0, 1].
     *
     * @return The interpolated value.
     */
    static T Linear(T from, T to, float t) noexcept
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return static_cast<T>(from + (to - from) * t);
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return static_cast<T>(from + static_cast<T>((to - from) * t + 0.5f));
        }
        else
        {
            return from + (to - from) * t;
        }
    }
};

} // namespace DragonOS::Animation
