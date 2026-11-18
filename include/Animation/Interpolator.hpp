

#pragma once

#include <concepts>
#include <type_traits>

namespace DragonOS::Animation {

template<typename T>
concept Interpolatable = std::is_arithmetic_v<T>;

template<typename T>
class Interpolator final {
public:
    Interpolator() = delete;

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

} 
