/**
 * @file    ThemeColor.hpp
 * @brief   RGBA colour value used throughout the theme system.
 *
 * ThemeColor is independent of any graphics API (Direct2D, GDI, etc.).
 * Conversion to API-specific types happens at the rendering layer.
 */

#pragma once

#include <cstdint>

namespace DragonOS::Theme {

/**
 * @brief  A floating-point RGBA colour in the [0, 1] range.
 *
 * All theme colours are stored as normalized floats to avoid
 * lossy conversions and to stay GPU-ready.
 */
struct ThemeColor final {
    float r{ 0.0f };  ///< Red   channel [0, 1].
    float g{ 0.0f };  ///< Green channel [0, 1].
    float b{ 0.0f };  ///< Blue  channel [0, 1].
    float a{ 1.0f };  ///< Alpha channel [0, 1].

    /// @brief  Create from 8-bit sRGB channels (0-255).
    [[nodiscard]] static constexpr ThemeColor FromRGB(
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue,
        std::uint8_t alpha = 255) noexcept
    {
        return {
            red   / 255.0f,
            green / 255.0f,
            blue  / 255.0f,
            alpha / 255.0f
        };
    }

    /// @brief  Create from normalized float channels.
    [[nodiscard]] static constexpr ThemeColor FromFloat(
        float red,
        float green,
        float blue,
        float alpha = 1.0f) noexcept
    {
        return { red, green, blue, alpha };
    }
};

} // namespace DragonOS::Theme
