/**
 * @file    Color.hpp
 * @brief   Lightweight RGBA colour representation.
 *
 * Provides a platform-independent colour type with a set of
 * commonly used predefined colours.
 */

#pragma once

namespace DragonOS::Graphics {

/**
 * @brief  RGBA colour with floating-point components in [0, 1].
 *
 * Components are stored in order: red, green, blue, alpha.
 * The default constructor produces opaque black.
 */
struct Color final {
    float r;   ///< Red   component [0, 1].
    float g;   ///< Green component [0, 1].
    float b;   ///< Blue  component [0, 1].
    float a;   ///< Alpha component [0, 1].

    /// @brief  Constructs opaque black.
    constexpr Color() noexcept : r{ 0.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f } {}

    /// @brief  Constructs a colour with explicit components.
    constexpr Color(float r, float g, float b, float a = 1.0f) noexcept
        : r{ r }, g{ g }, b{ b }, a{ a } {}

    // ── Predefined colours ─────────────────────────────────────────────
    // NOTE: Predefined colour constants are declared OUTSIDE the struct
    // (at namespace scope) so MSVC does not complain about using an
    // incomplete type within its own member-specification.
};

// ── Predefined colour constants (outside the struct for MSVC compat) ────

inline constexpr Color Black       { 0.00f, 0.00f, 0.00f };
inline constexpr Color White       { 1.00f, 1.00f, 1.00f };
inline constexpr Color Red         { 1.00f, 0.00f, 0.00f };
inline constexpr Color Green       { 0.00f, 1.00f, 0.00f };
inline constexpr Color Blue        { 0.00f, 0.00f, 1.00f };
inline constexpr Color Transparent { 0.00f, 0.00f, 0.00f, 0.00f };
inline constexpr Color DarkGray    { 0.25f, 0.25f, 0.25f };
inline constexpr Color LightGray   { 0.75f, 0.75f, 0.75f };
inline constexpr Color DragonBlack { 0.071f, 0.071f, 0.071f };

} // namespace DragonOS::Graphics
