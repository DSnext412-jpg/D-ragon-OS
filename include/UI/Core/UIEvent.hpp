#pragma once
#include <cstdint>
#include <Input/KeyCodes.hpp>
#include <Input/MouseButtons.hpp>
#include <functional>

namespace DragonOS::UI {

class UIElement;

enum class UIEventType : uint8_t {
    MouseEnter, MouseLeave, MouseMove,
    MouseDown, MouseUp, Click, DoubleClick,
    FocusGained, FocusLost,
    KeyDown, KeyUp, Char,
    ValueChanged, SelectionChanged,
    Scroll,
};

struct UIEvent {
    UIEventType type{UIEventType::MouseMove};
    float x{0}, y{0};
    Input::MouseButton button{Input::MouseButton::Left};
    Input::KeyCode key{Input::KeyCode::Unknown};
    wchar_t character{0};
    float wheelDelta{0};
    UIElement* target{nullptr};

    [[nodiscard]] bool IsMouseEvent() const noexcept;
    [[nodiscard]] bool IsKeyboardEvent() const noexcept;
    [[nodiscard]] bool IsFocusEvent() const noexcept;
};

using EventCallback = std::function<bool(UIElement&, const UIEvent&)>;

} // namespace
