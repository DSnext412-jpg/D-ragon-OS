/**
 * @file    Mouse.hpp
 * @brief   Mouse device state tracker.
 *
 * Mouse maintains the current and previous frame state of the
 * physical mouse — position, delta, wheel, and button states.
 * State is updated by InputManager in response to Win32 messages.
 */

#pragma once

#include <Input/MouseButtons.hpp>

#include <cstdint>

namespace DragonOS::Input {

/**
 * @brief  Tracks the frame-to-frame state of a physical mouse.
 *
 * Button state machine per frame:
 *   1. Messages arrive → raw button state updated immediately.
 *   2. Update() → pressed/released computed from raw transitions.
 *
 * Queries:
 *   IsButtonPressed()  — true only on the frame the button went down.
 *   IsButtonReleased() — true only on the frame the button went up.
 *   IsButtonHeld()     — true every frame the button is down.
 */
class Mouse final {
public:
    // ── Message-driven updates (called by InputManager) ──────────────────

    /// @brief  Record cursor movement.  @p x, @p y  are client-area DIPs.
    void OnMove(float x, float y) noexcept;

    /// @brief  Record that @p button was pressed.
    void OnButtonDown(MouseButton button) noexcept;

    /// @brief  Record that @p button was released.
    void OnButtonUp(MouseButton button) noexcept;

    /// @brief  Record a double-click on @p button.
    void OnDoubleClick(MouseButton button) noexcept;

    /// @brief  Accumulate wheel @p delta  (typically ±120 per notch).
    void OnWheel(float delta) noexcept;

    // ── Per-frame lifecycle ──────────────────────────────────────────────

    /**
     * @brief  Compute pressed / released state from raw transitions.
     *
     * Must be called exactly once per frame, before any query methods
     * are used for the current frame.
     */
    void Update() noexcept;

    /// @brief  Reset every button to the up state (e.g. on focus loss).
    void Reset() noexcept;

    // ── Queries ──────────────────────────────────────────────────────────

    [[nodiscard]] float GetX()         const noexcept { return m_x; }
    [[nodiscard]] float GetY()         const noexcept { return m_y; }
    [[nodiscard]] float GetDeltaX()    const noexcept { return m_deltaX; }
    [[nodiscard]] float GetDeltaY()    const noexcept { return m_deltaY; }
    [[nodiscard]] float GetWheelDelta() const noexcept { return m_wheelDelta; }

    /// @brief  True if @p button was pressed this frame.
    [[nodiscard]] bool IsButtonPressed(MouseButton button) const noexcept;

    /// @brief  True if @p button was released this frame.
    [[nodiscard]] bool IsButtonReleased(MouseButton button) const noexcept;

    /// @brief  True if @p button is currently held down.
    [[nodiscard]] bool IsButtonHeld(MouseButton button) const noexcept;

    /// @brief  True if @p button was double-clicked this frame.
    [[nodiscard]] bool IsButtonDoubleClicked(MouseButton button) const noexcept;

private:
    // ── Position ─────────────────────────────────────────────────────────
    float m_x{ 0.0f };
    float m_y{ 0.0f };
    float m_deltaX{ 0.0f };
    float m_deltaY{ 0.0f };

    // ── Wheel ────────────────────────────────────────────────────────────
    float m_wheelDelta{ 0.0f };
    bool  m_wheelUpdated{ false };

    // ── Button state (indexed by MouseButtonIndex) ───────────────────────
    bool m_rawState[MouseButtonCount]{};       ///< Physical state at all times.
    bool m_prevRawState[MouseButtonCount]{};   ///< Physical state at last Update().

    // Per-frame computed sets (updated in Update()).
    bool m_pressed[MouseButtonCount]{};
    bool m_released[MouseButtonCount]{};
    bool m_held[MouseButtonCount]{};
    bool m_doubleClicked[MouseButtonCount]{};
};

} // namespace DragonOS::Input
