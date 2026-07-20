/**
 * @file    WindowStyle.hpp
 * @brief   Bit-flag style attributes for DragonWindows.
 */

#pragma once

#include <cstdint>

namespace DragonOS::WindowManager {

/// @brief  Style flags that control a DragonWindow's behaviour and chrome.
enum class WindowStyle : uint8_t {
    None        = 0x00,
    Resizable   = 0x01,   ///< User can resize the window.
    Closable    = 0x02,   ///< Window has a close action.
    Movable     = 0x04,   ///< User can drag the window.
    Borderless  = 0x08,   ///< No chrome rendered.
    Transparent = 0x10,   ///< Back-buffer blending enabled.
};

// ── Bitwise operators ─────────────────────────────────────────────────────

constexpr WindowStyle operator|(WindowStyle a, WindowStyle b) noexcept
{
    return static_cast<WindowStyle>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr WindowStyle operator&(WindowStyle a, WindowStyle b) noexcept
{
    return static_cast<WindowStyle>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr WindowStyle operator~(WindowStyle a) noexcept
{
    return static_cast<WindowStyle>(~static_cast<uint8_t>(a));
}

constexpr WindowStyle& operator|=(WindowStyle& a, WindowStyle b) noexcept
{
    a = a | b;
    return a;
}

constexpr WindowStyle& operator&=(WindowStyle& a, WindowStyle b) noexcept
{
    a = a & b;
    return a;
}

constexpr bool HasFlag(WindowStyle value, WindowStyle flag) noexcept
{
    return (value & flag) == flag;
}

} // namespace DragonOS::WindowManager
