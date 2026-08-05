#pragma once

#include <cstdint>

namespace DragonOS::DragonUI {

/**
 * @brief  Bit-flag set describing the run-time state of an accessible element.
 *
 * Modelled after the UIA / MSAA element states so the states map onto a
 * future automation provider with minimal translation.  Use the helper
 * operators to compose states (e.g. Enabled | Focusable | Focused).
 */
enum class AccessibilityState : uint32_t {
    None           = 0x0000,
    Enabled        = 0x0001,  ///< The element accepts interaction.
    Disabled       = 0x0002,  ///< The element is disabled / unavailable.
    Focusable      = 0x0004,  ///< The element can receive keyboard focus.
    Focused        = 0x0008,  ///< The element currently has keyboard focus.
    Selected       = 0x0010,  ///< The element is selected.
    SelectedInactive = 0x0020, ///< Selected but not active.
    Checked        = 0x0040,  ///< Check box / radio is checked (on).
    Unchecked      = 0x0080,  ///< Check box / radio is unchecked (off).
    Indeterminate  = 0x0100,  ///< Mixed / indeterminate state.
    Pressed        = 0x0200,  ///< Currently being pressed.
    Expanded       = 0x0400,  ///< Tree/list node expanded.
    Collapsed      = 0x0800,  ///< Tree/list node collapsed.
    ReadOnly       = 0x1000,  ///< Read-only (e.g. read-only text box).
    Invalid        = 0x2000,  ///< Validation / entry error.
    OffScreen      = 0x4000,  ///< Present but outside the visible viewport.
    Protected      = 0x8000,  ///< Content is masked (password).
    Busy           = 0x10000, ///< Asynchronous operation in progress.
    Visible        = 0x20000, ///< The element is visible in the tree.
};

inline constexpr AccessibilityState operator|(AccessibilityState a, AccessibilityState b) noexcept
{
    return static_cast<AccessibilityState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr AccessibilityState& operator|=(AccessibilityState& a, AccessibilityState b) noexcept
{
    a = a | b;
    return a;
}

inline constexpr bool HasState(AccessibilityState value, AccessibilityState flag) noexcept
{
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

} // namespace DragonOS::DragonUI
