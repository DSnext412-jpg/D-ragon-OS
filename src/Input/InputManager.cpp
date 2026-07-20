/**
 * @file    InputManager.cpp
 * @brief   Implementation of the InputManager class.
 */

#include <Input/InputManager.hpp>

#include <Windows.h>
#include <windowsx.h>

namespace DragonOS::Input {

// ============================================================================
//  Per-frame lifecycle
// ============================================================================

void InputManager::Update(float /*deltaTime*/) noexcept
{
    // Compute pressed / released / held from the raw state transitions
    // that occurred during the previous message-batch.
    m_mouse.Update();
    m_mouseManager.Update();
    m_keyboard.Update();

    // The event queue has been available for reading during this Update
    // call; now it is safe to clear for the next frame.
    m_eventQueue.clear();
}

// ============================================================================
//  Win32 message sink
// ============================================================================

void InputManager::HandleWin32Message(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    // ── Mouse ────────────────────────────────────────────────────────────
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
        HandleMouseMessage(uMsg, wParam, lParam);
        return;

    // ── Keyboard ─────────────────────────────────────────────────────────
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        HandleKeyMessage(uMsg, wParam, lParam);
        return;

    case WM_CHAR:
    {
        const auto ch = static_cast<wchar_t>(wParam);
        PushCharEvent(ch);
        return;
    }

    // ── Ignore all other messages ────────────────────────────────────────
    default:
        return;
    }
}

// ============================================================================
//  Mouse message handler
// ============================================================================

void InputManager::HandleMouseMessage(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    // ── Position ─────────────────────────────────────────────────────────
    // WM_MOUSEWHEEL LPARAM contains screen coords; we use the last known
    // client position instead.
    const float clientX = (uMsg == WM_MOUSEWHEEL)
                              ? m_mouse.GetX()
                              : static_cast<float>(GET_X_LPARAM(lParam));

    const float clientY = (uMsg == WM_MOUSEWHEEL)
                              ? m_mouse.GetY()
                              : static_cast<float>(GET_Y_LPARAM(lParam));

    // Determine which button (if any) is involved.
    MouseButton btn      = MouseButton::Left;  // default
    bool        isX      = false;
    bool        isWheel  = false;

    switch (uMsg)
    {
    case WM_MOUSEMOVE:
        m_mouse.OnMove(clientX, clientY);
        m_mouseManager.OnMove(clientX, clientY);
        PushMoveEvent(clientX, clientY);
        return;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        btn = MouseButton::Left;
        break;

    case WM_LBUTTONUP:
        btn = MouseButton::Left;
        m_mouse.OnButtonUp(btn);
        m_mouseManager.OnButtonUp(btn);
        PushButtonEvent(EventType::MouseUp, btn, clientX, clientY);
        return;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
        btn = MouseButton::Right;
        break;

    case WM_RBUTTONUP:
        btn = MouseButton::Right;
        m_mouse.OnButtonUp(btn);
        m_mouseManager.OnButtonUp(btn);
        PushButtonEvent(EventType::MouseUp, btn, clientX, clientY);
        return;

    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
        btn = MouseButton::Middle;
        break;

    case WM_MBUTTONUP:
        btn = MouseButton::Middle;
        m_mouse.OnButtonUp(btn);
        m_mouseManager.OnButtonUp(btn);
        PushButtonEvent(EventType::MouseUp, btn, clientX, clientY);
        return;

    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONUP:
        isX = true;
        // XBUTTON1 vs XBUTTON2 is in HIWORD(wParam).
        btn = (HIWORD(wParam) == XBUTTON1)
                  ? MouseButton::XButton1
                  : MouseButton::XButton2;

        if (uMsg == WM_XBUTTONUP)
        {
            m_mouse.OnButtonUp(btn);
            m_mouseManager.OnButtonUp(btn);
            PushButtonEvent(EventType::MouseUp, btn, clientX, clientY);
            return;
        }
        break;

    case WM_MOUSEWHEEL:
        isWheel = true;
        {
            const float delta = static_cast<SHORT>(HIWORD(wParam));
            m_mouse.OnWheel(delta);
            m_mouseManager.OnWheel(delta);
            PushWheelEvent(delta);
        }
        return;

    default:
        return;
    }

    // ── Button-down / double-click path ──────────────────────────────────
    if (isWheel) { return; }

    const bool isDblClick = (uMsg == WM_LBUTTONDBLCLK ||
                             uMsg == WM_RBUTTONDBLCLK ||
                             uMsg == WM_MBUTTONDBLCLK ||
                             uMsg == WM_XBUTTONDBLCLK);

    if (isDblClick)
    {
        m_mouse.OnDoubleClick(btn);
        m_mouseManager.OnDoubleClick(btn);
        PushButtonEvent(EventType::MouseDoubleClick, btn, clientX, clientY);
    }
    else
    {
        m_mouse.OnButtonDown(btn);
        m_mouseManager.OnButtonDown(btn);
    }

    PushButtonEvent(EventType::MouseDown, btn, clientX, clientY);
}

// ============================================================================
//  Keyboard message handler
// ============================================================================

void InputManager::OnMouseLeave() noexcept
{
    m_mouseManager.OnLeave();
}

void InputManager::HandleKeyMessage(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam) noexcept
{
    const auto key   = static_cast<KeyCode>(wParam);
    const bool isDown  = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
    const bool isRepeat = (lParam & 0x40000000) != 0;

    if (isDown)
    {
        m_keyboard.OnKeyDown(key);
        PushKeyEvent(EventType::KeyDown, key, isRepeat);
    }
    else
    {
        m_keyboard.OnKeyUp(key);
        PushKeyEvent(EventType::KeyUp, key, false);
    }
}

// ============================================================================
//  Event helpers
// ============================================================================

void InputManager::PushMoveEvent(float x, float y) noexcept
{
    InputEvent ev{};
    ev.type            = EventType::MouseMove;
    ev.data.mouseMove  = { x, y, m_mouse.GetDeltaX(), m_mouse.GetDeltaY() };
    m_eventQueue.push_back(ev);
}

void InputManager::PushButtonEvent(
    EventType    type,
    MouseButton  button,
    float        x,
    float        y) noexcept
{
    InputEvent ev{};
    ev.type               = type;
    ev.data.mouseButton   = { button, x, y };
    m_eventQueue.push_back(ev);
}

void InputManager::PushWheelEvent(float delta) noexcept
{
    InputEvent ev{};
    ev.type             = EventType::MouseWheel;
    ev.data.mouseWheel  = { delta, m_mouse.GetX(), m_mouse.GetY() };
    m_eventQueue.push_back(ev);
}

void InputManager::PushKeyEvent(
    EventType type,
    KeyCode   key,
    bool      isRepeat) noexcept
{
    InputEvent ev{};
    ev.type        = type;
    ev.data.key    = { key, isRepeat };
    m_eventQueue.push_back(ev);
}

void InputManager::PushCharEvent(wchar_t ch) noexcept
{
    InputEvent ev{};
    ev.type             = EventType::CharacterInput;
    ev.data.character   = { ch };
    m_eventQueue.push_back(ev);
}

} // namespace DragonOS::Input
