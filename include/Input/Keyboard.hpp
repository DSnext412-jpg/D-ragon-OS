/**
 * @file    Keyboard.hpp
 * @brief   Keyboard device state tracker.
 *
 * Keyboard maintains the current and previous frame state of the
 * physical keyboard — pressed, released, held keys, and modifier
 * flags (Ctrl, Shift, Alt, Win).  State is updated by InputManager
 * in response to Win32 messages.
 */

#pragma once

#include <Input/KeyCodes.hpp>

#include <cstdint>

namespace DragonOS::Input {

/**
 * @brief  Tracks the frame-to-frame state of a physical keyboard.
 *
 * Key state machine per frame:
 *   1. Messages arrive → raw key state updated immediately.
 *   2. Update() → pressed/released computed from raw transitions.
 *   3. Modifier flags refreshed from raw key state.
 *
 * Queries:
 *   IsKeyPressed()  — true only on the frame the key went down.
 *   IsKeyReleased() — true only on the frame the key went up.
 *   IsKeyHeld()     — true every frame the key is down.
 */
class Keyboard final {
public:
    // ── Message-driven updates (called by InputManager) ──────────────────

    /// @brief  Record that @p key was pressed.
    void OnKeyDown(KeyCode key) noexcept;

    /// @brief  Record that @p key was released.
    void OnKeyUp(KeyCode key) noexcept;

    // ── Per-frame lifecycle ──────────────────────────────────────────────

    /**
     * @brief  Compute pressed / released state from raw transitions
     *         and refresh modifier flags.
     *
     * Must be called exactly once per frame, before any query methods
     * are used for the current frame.
     */
    void Update() noexcept;

    /// @brief  Release every key (e.g. on focus loss).
    void Reset() noexcept;

    // ── Queries ──────────────────────────────────────────────────────────

    /// @brief  True if @p key was pressed this frame (first frame down).
    [[nodiscard]] bool IsKeyPressed(KeyCode key) const noexcept;

    /// @brief  True if @p key was released this frame (first frame up).
    [[nodiscard]] bool IsKeyReleased(KeyCode key) const noexcept;

    /// @brief  True if @p key is currently held down.
    [[nodiscard]] bool IsKeyHeld(KeyCode key) const noexcept;

    // ── Modifier helpers ─────────────────────────────────────────────────

    [[nodiscard]] bool IsCtrlDown()  const noexcept { return m_ctrlDown; }
    [[nodiscard]] bool IsShiftDown() const noexcept { return m_shiftDown; }
    [[nodiscard]] bool IsAltDown()   const noexcept { return m_altDown; }
    [[nodiscard]] bool IsWinDown()   const noexcept { return m_winDown; }

private:
    void UpdateModifiers() noexcept;

    // ── Key state arrays (indexed by KeyCode value) ──────────────────────
    bool m_rawState[KeyCodeCount]{};       ///< Physical state at all times.
    bool m_prevRawState[KeyCodeCount]{};   ///< Physical state at last Update().

    // Per-frame computed sets (updated in Update()).
    bool m_pressed[KeyCodeCount]{};
    bool m_released[KeyCodeCount]{};
    bool m_held[KeyCodeCount]{};

    // ── Modifier flags ───────────────────────────────────────────────────
    bool m_ctrlDown{ false };
    bool m_shiftDown{ false };
    bool m_altDown{ false };
    bool m_winDown{ false };
};

} // namespace DragonOS::Input
