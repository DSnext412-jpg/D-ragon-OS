#pragma once

#include <Input/MouseButtons.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>

namespace DragonOS::Input {

class MouseManager final {
public:
    MouseManager() noexcept = default;

    MouseManager(const MouseManager&)            = delete;
    MouseManager& operator=(const MouseManager&) = delete;

    // ── Message-driven updates (called by InputManager) ──────────────────
    void OnMove(float x, float y) noexcept;
    void OnButtonDown(MouseButton button) noexcept;
    void OnButtonUp(MouseButton button) noexcept;
    void OnDoubleClick(MouseButton button) noexcept;
    void OnWheel(float delta) noexcept;
    void OnLeave() noexcept;

    // ── Per-frame lifecycle ──────────────────────────────────────────────
    void Update() noexcept;
    void Reset() noexcept;

    // ── Position ─────────────────────────────────────────────────────────
    [[nodiscard]] Point GetPosition()        const noexcept { return { m_x, m_y }; }
    [[nodiscard]] Point GetPreviousPosition() const noexcept { return { m_prevX, m_prevY }; }
    [[nodiscard]] float GetX()               const noexcept { return m_x; }
    [[nodiscard]] float GetY()               const noexcept { return m_y; }
    [[nodiscard]] float GetDeltaX()          const noexcept { return m_deltaX; }
    [[nodiscard]] float GetDeltaY()          const noexcept { return m_deltaY; }
    [[nodiscard]] Point GetDelta()           const noexcept { return { m_deltaX, m_deltaY }; }

    // ── Buttons ──────────────────────────────────────────────────────────
    [[nodiscard]] bool IsPressed(MouseButton btn)      const noexcept;
    [[nodiscard]] bool WasClicked(MouseButton btn)     const noexcept;
    [[nodiscard]] bool WasReleased(MouseButton btn)    const noexcept;
    [[nodiscard]] bool IsHeld(MouseButton btn)         const noexcept;
    [[nodiscard]] bool WasDoubleClicked(MouseButton btn) const noexcept;

    // Convenience
    [[nodiscard]] bool IsLeftPressed()   const noexcept { return IsPressed(MouseButton::Left); }
    [[nodiscard]] bool WasLeftClicked()  const noexcept { return WasClicked(MouseButton::Left); }
    [[nodiscard]] bool IsRightPressed()  const noexcept { return IsPressed(MouseButton::Right); }
    [[nodiscard]] bool WasRightClicked() const noexcept { return WasClicked(MouseButton::Right); }

    // ── Wheel ────────────────────────────────────────────────────────────
    [[nodiscard]] float GetWheelDelta() const noexcept { return m_wheelDelta; }

    // ── Drag ─────────────────────────────────────────────────────────────
    [[nodiscard]] bool IsDragging() const noexcept { return m_isDragging; }
    [[nodiscard]] Point GetDragStart() const noexcept { return { m_dragStartX, m_dragStartY }; }
    [[nodiscard]] MouseButton GetDragButton() const noexcept { return m_dragButton; }

    // ── Hover ────────────────────────────────────────────────────────────
    [[nodiscard]] bool IsInClient() const noexcept { return m_inClient; }

private:
    static constexpr float DragThreshold = 4.0f;

    float m_x{ 0.0f }, m_y{ 0.0f };
    float m_prevX{ 0.0f }, m_prevY{ 0.0f };
    float m_deltaX{ 0.0f }, m_deltaY{ 0.0f };
    float m_wheelDelta{ 0.0f };
    bool  m_wheelUpdated{ false };
    bool  m_inClient{ false };

    bool  m_isDragging{ false };
    float m_dragStartX{ 0.0f }, m_dragStartY{ 0.0f };
    MouseButton m_dragButton{ MouseButton::Left };

    bool m_rawState[MouseButtonCount]{};
    bool m_prevRawState[MouseButtonCount]{};
    bool m_pressed[MouseButtonCount]{};
    bool m_released[MouseButtonCount]{};
    bool m_held[MouseButtonCount]{};
    bool m_doubleClicked[MouseButtonCount]{};
};

}
