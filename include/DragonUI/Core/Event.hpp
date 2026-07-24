#pragma once

#include <Input/KeyCodes.hpp>
#include <Input/MouseButtons.hpp>
#include <cstdint>

namespace DragonOS::DragonUI {

class Element;

enum class EventType : uint8_t {
    MouseEnter,
    MouseLeave,
    MouseMove,
    MouseDown,
    MouseUp,
    Click,
    DoubleClick,
    GotFocus,
    LostFocus,
    KeyDown,
    KeyUp,
    TextInput,
};

struct MouseEventArgs {
    float x{};
    float y{};
    Input::MouseButton button{Input::MouseButton::Left};
    int clickCount{1};
    float wheelDelta{};
};

struct KeyEventArgs {
    Input::KeyCode key{Input::KeyCode::Unknown};
    wchar_t character{};
    bool isRepeat{};
    bool ctrl{};
    bool shift{};
    bool alt{};
};

struct FocusEventArgs {
    Element* previous{};
    Element* current{};
};

struct EventArgs {
    EventType type{};
    union {
        MouseEventArgs mouse;
        KeyEventArgs key;
        FocusEventArgs focus;
    };

    EventArgs() { /* union default */ }

    static EventArgs MakeMouse(EventType t, float x, float y, Input::MouseButton btn = Input::MouseButton::Left, int clicks = 1) {
        EventArgs e;
        e.type = t;
        e.mouse = {x, y, btn, clicks};
        return e;
    }

    static EventArgs MakeKey(EventType t, Input::KeyCode k, wchar_t ch = {}, bool rep = false) {
        EventArgs e;
        e.type = t;
        e.key = {k, ch, rep};
        return e;
    }

    static EventArgs MakeFocus(Element* prev, Element* cur) {
        EventArgs e;
        e.type = EventType::GotFocus;
        e.focus = {prev, cur};
        return e;
    }
};

} // namespace
