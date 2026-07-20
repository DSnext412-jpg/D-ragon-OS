/**
 * @file    InputManager.hpp
 * @brief   Central input coordinator — the single source of all
 *          keyboard and mouse input in DragonOS.
 *
 * InputManager is both:
 *   - An Engine::System (registered with SystemManager for lifecycle).
 *   - The Win32 message sink for every input message the main Window
 *     receives.
 *
 * Future modules must never read Win32 input messages directly.
 * Instead they query InputManager or subscribe to its event queue.
 */

#pragma once

#include <Engine/System.hpp>

#include <Input/InputEvent.hpp>
#include <Input/Keyboard.hpp>
#include <Input/Mouse.hpp>

#include <Windows.h>

#include <vector>

namespace DragonOS::Input {

/**
 * @brief  Drives Mouse and Keyboard state machines and manages the
 *         per-frame input event queue.
 *
 * ## Lifecycle (Engine::System)
 *   - Initialize() — no-op, returns true.
 *   - Update()     — processes event queue, updates pressed/released
 *                    state for mouse and keyboard.
 *   - Shutdown()   — no-op.
 *   - Render()     — no-op (input does not draw).
 *   - Resize()     — no-op.
 *
 * ## Win32 integration
 * Window forwards relevant WM_* messages to HandleWin32Message().
 * InputManager decodes and dispatches to Mouse / Keyboard.
 *
 * ## Public API
 * Clients query state via the convenience methods or by accessing
 * the underlying Mouse / Keyboard objects directly.
 */
class InputManager final : public Engine::System {
public:
    // ── Engine::System ───────────────────────────────────────────────────

    [[nodiscard]] bool Initialize(Engine::EngineContext& /*ctx*/) noexcept override
    {
        return true;
    }

    void Shutdown() noexcept override {}

    /**
     * @brief  Per-frame update.
     *
     * 1. Computes per-frame pressed / released state for mouse + keyboard.
     * 2. Clears the per-frame event queue.
     *
     * @param deltaTime  Seconds elapsed since previous frame (unused).
     */
    void Update(float deltaTime) noexcept override;

    /// @brief  No-op — input does not render.
    void Render(Engine::EngineContext& /*ctx*/) noexcept override {}

    /// @brief  No-op — input does not respond to viewport size changes.
    void Resize(float /*width*/, float /*height*/) noexcept override {}

    // ── Win32 message sink ───────────────────────────────────────────────

    /**
     * @brief  Decode a Win32 input message and update internal state.
     *
     * Called by Window for every relevant WM_* message.
     *
     * @param uMsg    Win32 message identifier.
     * @param wParam  Message WPARAM.
     * @param lParam  Message LPARAM.
     */
    void HandleWin32Message(
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam) noexcept;

    // ── Device access ────────────────────────────────────────────────────

    [[nodiscard]] Mouse&          GetMouse()       noexcept { return m_mouse; }
    [[nodiscard]] const Mouse&    GetMouse() const noexcept { return m_mouse; }
    [[nodiscard]] Keyboard&       GetKeyboard()       noexcept { return m_keyboard; }
    [[nodiscard]] const Keyboard& GetKeyboard() const noexcept { return m_keyboard; }

    // ── Event queue ──────────────────────────────────────────────────────

    /// @brief  Read-only access to the current frame's event queue.
    [[nodiscard]] const std::vector<InputEvent>& GetEvents() const noexcept
    {
        return m_eventQueue;
    }

    // ── Convenience queries ──────────────────────────────────────────────

    [[nodiscard]] bool IsKeyPressed(KeyCode k)  const noexcept { return m_keyboard.IsKeyPressed(k); }
    [[nodiscard]] bool IsKeyHeld(KeyCode k)     const noexcept { return m_keyboard.IsKeyHeld(k); }
    [[nodiscard]] bool IsKeyReleased(KeyCode k) const noexcept { return m_keyboard.IsKeyReleased(k); }

    [[nodiscard]] bool IsMousePressed(MouseButton b)  const noexcept { return m_mouse.IsButtonPressed(b); }
    [[nodiscard]] bool IsMouseHeld(MouseButton b)     const noexcept { return m_mouse.IsButtonHeld(b); }
    [[nodiscard]] bool IsMouseReleased(MouseButton b) const noexcept { return m_mouse.IsButtonReleased(b); }

    [[nodiscard]] float GetMouseX()         const noexcept { return m_mouse.GetX(); }
    [[nodiscard]] float GetMouseY()         const noexcept { return m_mouse.GetY(); }
    [[nodiscard]] float GetMouseDeltaX()    const noexcept { return m_mouse.GetDeltaX(); }
    [[nodiscard]] float GetMouseDeltaY()    const noexcept { return m_mouse.GetDeltaY(); }
    [[nodiscard]] float GetWheelDelta()     const noexcept { return m_mouse.GetWheelDelta(); }

private:
    // ── Internal helpers ─────────────────────────────────────────────────

    void PushMoveEvent(float x, float y) noexcept;
    void PushButtonEvent(EventType type, MouseButton button, float x, float y) noexcept;
    void PushWheelEvent(float delta) noexcept;
    void PushKeyEvent(EventType type, KeyCode key, bool isRepeat) noexcept;
    void PushCharEvent(wchar_t ch) noexcept;

    void HandleMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
    void HandleKeyMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

    // ── Data members ─────────────────────────────────────────────────────

    Mouse                   m_mouse;
    Keyboard                m_keyboard;
    std::vector<InputEvent> m_eventQueue;
};

} // namespace DragonOS::Input
