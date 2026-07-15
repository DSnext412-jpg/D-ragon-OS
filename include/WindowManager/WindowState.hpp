/**
 * @file    WindowState.hpp
 * @brief   Enumeration of all possible DragonWindow visibility states.
 */

#pragma once

#include <cstdint>

namespace DragonOS::WindowManager {

/// @brief  Current presentation state of a DragonWindow.
enum class WindowState : uint8_t {
    Normal,     ///< Default restored state.
    Minimized,  ///< Collapsed (not rendered).
    Maximized,  ///< Full client-area size.
    Hidden,     ///< Invisible.
};

} // namespace DragonOS::WindowManager
