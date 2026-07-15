/**
 * @file    InputEvent.hpp
 * @brief   Event structures for the DragonOS input system.
 *
 * Every input action (mouse move, button press, key down, etc.) is
 * represented as an InputEvent and stored in InputManager's event
 * queue for per-frame processing.
 */

#pragma once

#include <Input/KeyCodes.hpp>
#include <Input/MouseButtons.hpp>

#include <cstdint>

namespace DragonOS::Input {

// ── Event type enumeration ────────────────────────────────────────────────

/**
 * @brief  Discriminator for the kind of input event.
 */
enum class EventType : uint32_t {
    MouseMove,         ///< Cursor moved.
    MouseDown,         ///< Mouse button pressed.
    MouseUp,           ///< Mouse button released.
    MouseDoubleClick,  ///< Mouse button double-clicked.
    MouseWheel,        ///< Scroll wheel rotated.
    KeyDown,           ///< Key pressed.
    KeyUp,             ///< Key released.
    CharacterInput,    ///< Character received via WM_CHAR.

    // Future input types (TODO — not yet implemented).
    Touch = 0x10,      ///< Touch event (reserved).
    Stylus,            ///< Pen / stylus event (reserved).
    Gamepad,           ///< Gamepad event (reserved).
    RawInput,          ///< Raw-input event (reserved).
};

// ── Per-type event data ───────────────────────────────────────────────────

/**
 * @brief  Data for a MouseMove event.
 */
struct MouseMoveEvent final {
    float x;        ///< Client-space X.
    float y;        ///< Client-space Y.
    float deltaX;   ///< Change in X since last move.
    float deltaY;   ///< Change in Y since last move.
};

/**
 * @brief  Data for a mouse button event (MouseDown, MouseUp,
 *         MouseDoubleClick).
 */
struct MouseButtonEvent final {
    MouseButton button; ///< Which button.
    float       x;      ///< Client-space X at time of event.
    float       y;      ///< Client-space Y at time of event.
};

/**
 * @brief  Data for a MouseWheel event.
 */
struct MouseWheelEvent final {
    float delta;    ///< Wheel movement (positive = forward/up).
    float x;        ///< Client-space X.
    float y;        ///< Client-space Y.
};

/**
 * @brief  Data for a KeyDown or KeyUp event.
 */
struct KeyEvent final {
    KeyCode key;        ///< The virtual key.
    bool    isRepeat;   ///< true if this is an auto-repeated KeyDown.
};

/**
 * @brief  Data for a CharacterInput event.
 */
struct CharacterEvent final {
    wchar_t character;  ///< The UTF-16 code unit.
};

// ── Tagged union event ────────────────────────────────────────────────────

/**
 * @brief  A single input event with a type tag and type-specific data.
 */
struct InputEvent final {
    EventType type; ///< Identifies which member of @ref data is active.

    union {
        MouseMoveEvent    mouseMove;
        MouseButtonEvent  mouseButton;
        MouseWheelEvent   mouseWheel;
        KeyEvent          key;
        CharacterEvent    character;
    } data;           ///< Type-specific payload.
};

} // namespace DragonOS::Input
