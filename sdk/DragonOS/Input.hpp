#pragma once

#include <cstdint>

namespace dragonos::sdk {

enum class KeyCode : uint8_t {
    None,
    Left, Right, Up, Down,
    Return, Escape, Space, Tab, Backspace,
    Delete, Insert, Home, End, PageUp, PageDown,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LCtrl, RCtrl, LShift, RShift, LAlt, RAlt, LWin, RWin,
};

enum class MouseButton : uint8_t { None, Left, Right, Middle, X1, X2 };

struct MouseState {
    float x{ 0.0f }, y{ 0.0f };
    float deltaX{ 0.0f }, deltaY{ 0.0f };
    float wheelDelta{ 0.0f };
    bool leftPressed{ false };
    bool rightPressed{ false };
    bool leftClicked{ false };
    bool rightClicked{ false };
};

struct KeyEvent {
    KeyCode key{ KeyCode::None };
    bool pressed{ false };
    bool released{ false };
    bool ctrl{ false };
    bool shift{ false };
    bool alt{ false };
};

enum class InputEventType {
    KeyDown, KeyUp, MouseMove, MouseDown, MouseUp, MouseWheel, Char,
};

struct InputEventData {
    InputEventType type{ InputEventType::KeyDown };
    KeyCode key{ KeyCode::None };
    wchar_t character{ 0 };
    float mouseX{ 0.0f }, mouseY{ 0.0f };
};

class IInputService {
public:
    virtual ~IInputService() noexcept = default;
    virtual MouseState GetMouseState() const noexcept = 0;
    virtual bool IsKeyDown(KeyCode key) const noexcept = 0;
    virtual bool WasKeyPressed(KeyCode key) const noexcept = 0;
    virtual bool WasKeyReleased(KeyCode key) const noexcept = 0;
    virtual size_t GetEventCount() const noexcept = 0;
    virtual InputEventData GetEvent(size_t index) const noexcept = 0;
};

} // namespace dragonos::sdk
