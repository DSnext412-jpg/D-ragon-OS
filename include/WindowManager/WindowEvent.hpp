/**
 * @file    WindowEvent.hpp
 * @brief   Event types and payload for the DragonWindow system.
 *
 * Reserved for future interactivity (dragging, resizing, snapping,
 * docking, animation triggers).
 */

#pragma once

#include <cstdint>

namespace DragonOS::WindowManager {

/// @brief  Categories of window events.
enum class WindowEventType : uint8_t {
    None,
    Close,
    Minimize,
    Maximize,
    Restore,
    Move,
    Resize,
    GainFocus,
    LoseFocus,
};

/**
 * @brief  A single event raised by or sent to a DragonWindow.
 *
 * The payload union will be extended when drag/resize/snap/dock
 * interactions are implemented.
 */
struct WindowEvent final {
    WindowEventType type{ WindowEventType::None };
};

} // namespace DragonOS::WindowManager
