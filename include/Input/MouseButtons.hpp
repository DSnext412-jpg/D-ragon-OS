/**
 * @file    MouseButtons.hpp
 * @brief   Mouse button identifier for DragonOS input.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace DragonOS::Input {

/**
 * @brief  Identifies a physical mouse button.
 */
enum class MouseButton : uint32_t {
    Left     = 0,   ///< Primary button (typically left).
    Right    = 1,   ///< Secondary button (typically right).
    Middle   = 2,   ///< Scroll-wheel button.
    XButton1 = 3,   ///< First extended button (side button).
    XButton2 = 4,   ///< Second extended button.

    Count_   = 5,   ///< Internal sentinel — not a valid button.
};

/**
 * @brief  Number of distinct mouse buttons.
 */
inline constexpr std::size_t MouseButtonCount =
    static_cast<std::size_t>(MouseButton::Count_);

/**
 * @brief  Convert a MouseButton to its underlying zero-based index.
 */
[[nodiscard]] inline constexpr std::uint32_t MouseButtonIndex(MouseButton btn) noexcept
{
    return static_cast<std::uint32_t>(btn);
}

} // namespace DragonOS::Input
