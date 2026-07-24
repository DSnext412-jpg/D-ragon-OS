#pragma once

#include <DragonUI/Core/Element.hpp>

namespace DragonOS::DragonUI {

enum class ControlState : uint8_t {
    Normal,
    Hover,
    Pressed,
    Focused,
    Disabled,
};

class Control : public Element {
public:
    Control() noexcept = default;

    [[nodiscard]] virtual bool OnEvent(const EventArgs& args) noexcept;

    void SetFocusable(bool focusable) noexcept { m_focusable = focusable; }
    [[nodiscard]] bool IsFocusable() const noexcept { return m_focusable; }

    void SetTabIndex(int index) noexcept { m_tabIndex = index; }
    [[nodiscard]] int GetTabIndex() const noexcept { return m_tabIndex; }

    [[nodiscard]] ControlState GetControlState() const noexcept { return m_controlState; }
    void SetControlState(ControlState state) noexcept { m_controlState = state; InvalidateVisual(); }

    [[nodiscard]] virtual bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept;
    [[nodiscard]] virtual bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept;
    [[nodiscard]] virtual bool OnFocusEvent(const FocusEventArgs& args) noexcept;

protected:
    bool m_focusable{};
    int m_tabIndex{};
    ControlState m_controlState{ControlState::Normal};
};

} // namespace
