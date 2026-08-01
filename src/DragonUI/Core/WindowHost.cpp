#include <DragonUI/Core/WindowHost.hpp>
#include <DragonUI/Dialogs/DialogManager.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

WindowHost::WindowHost(
    Graphics::Renderer& renderer,
    const Theme::ThemeManager& theme,
    Input::InputManager& input) noexcept
    : m_renderer(renderer)
    , m_theme(theme)
    , m_input(input)
    , m_dialogs(std::make_unique<DialogManager>())
{
}

WindowHost::~WindowHost() noexcept
{
    if (m_root)
    {
        m_focusMgr.UnregisterRoot(dynamic_cast<Container*>(m_root.get()));
    }
}

void WindowHost::SetRoot(std::unique_ptr<Element> root) noexcept
{
    if (m_root)
        m_focusMgr.UnregisterRoot(dynamic_cast<Container*>(m_root.get()));

    m_root = std::move(root);

    if (auto* container = dynamic_cast<Container*>(m_root.get()))
        m_focusMgr.RegisterRoot(container);
}

void WindowHost::SetDpiScale(float scale) noexcept
{
    m_dpiScale = scale;
    if (m_root)
        m_root->InvalidateLayout();
}

void WindowHost::Resize(float width, float height) noexcept
{
    m_viewportW = width;
    m_viewportH = height;
    if (m_root)
        m_root->InvalidateLayout();
    if (m_dialogs)
        m_dialogs->SetViewport(width, height);
}

void WindowHost::Update(float deltaTime) noexcept
{
    if (m_root)
    {
        LayoutSlot viewport{0, 0, m_viewportW, m_viewportH};
        m_root->Measure(viewport);
        m_root->Arrange(viewport);
    }

    if (m_dialogs)
        m_dialogs->Update(deltaTime, m_focusMgr);
}

void WindowHost::Render() noexcept
{
    if (!m_root) return;

    RenderContext ctx(m_renderer, m_theme, m_dpiScale);
    m_root->Render(ctx);

    if (m_dialogs && !m_dialogs->IsEmpty())
        m_dialogs->Render(ctx, m_viewportW, m_viewportH);
}

// ── Hit testing ──────────────────────────────────────────────────────

Control* WindowHost::HitTestControl(float x, float y) noexcept
{
    if (!m_root) return nullptr;
    auto* hit = m_root->HitTest(x, y);
    return dynamic_cast<Control*>(hit);
}

void WindowHost::DispatchEvent(Control* target, const EventArgs& args) noexcept
{
    if (!target) return;
    (void)target->OnEvent(args);
}

// ── Hover tracking ───────────────────────────────────────────────────

void WindowHost::UpdateHover(float x, float y) noexcept
{
    auto* newHover = HitTestControl(x, y);

    if (newHover != m_hovered)
    {
        if (m_hovered)
        {
            m_hovered->SetControlState(ControlState::Normal);
            DispatchEvent(m_hovered, EventArgs::MakeMouse(EventType::MouseLeave, x, y));
        }

        m_hovered = newHover;

        if (m_hovered)
        {
            m_hovered->SetControlState(ControlState::Hover);
            DispatchEvent(m_hovered, EventArgs::MakeMouse(EventType::MouseEnter, x, y));
        }
    }

    if (m_hovered)
    {
        DispatchEvent(m_hovered, EventArgs::MakeMouse(EventType::MouseMove, x, y));
        m_hovered->SetControlState(
            m_pressed == m_hovered ? ControlState::Pressed : ControlState::Hover);
    }
}

// ── Input handlers ───────────────────────────────────────────────────

namespace {
bool IsCtrlHeld(const Input::InputManager& input) noexcept
{
    return input.IsKeyHeld(Input::KeyCode::LControl) || input.IsKeyHeld(Input::KeyCode::RControl);
}
bool IsShiftHeld(const Input::InputManager& input) noexcept
{
    return input.IsKeyHeld(Input::KeyCode::LShift) || input.IsKeyHeld(Input::KeyCode::RShift);
}
bool IsAltHeld(const Input::InputManager& input) noexcept
{
    return input.IsKeyHeld(Input::KeyCode::LAlt) || input.IsKeyHeld(Input::KeyCode::RAlt);
}
} // namespace

void WindowHost::OnMouseMove(float x, float y) noexcept
{
    if (!m_root) return;
    if (m_dialogs && m_dialogs->HandleMouseMove(x, y)) return;
    UpdateHover(x, y);
}

void WindowHost::OnMouseDown(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_root) return;
    if (m_dialogs && m_dialogs->HandleMouseDown(x, y, button)) return;

    m_pressed = HitTestControl(x, y);

    if (m_pressed)
    {
        m_pressed->SetControlState(ControlState::Pressed);

        if (m_pressed->IsFocusable())
            m_focusMgr.SetFocus(m_pressed);

        auto args = EventArgs::MakeMouse(EventType::MouseDown, x, y, button);
        args.mouse.ctrl = IsCtrlHeld(m_input);
        args.mouse.shift = IsShiftHeld(m_input);
        args.mouse.alt = IsAltHeld(m_input);
        DispatchEvent(m_pressed, args);
    }
}

void WindowHost::OnMouseUp(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_root) return;
    if (m_dialogs && m_dialogs->HandleMouseUp(x, y, button)) return;
    if (!m_pressed) return;

    auto* releaseTarget = m_pressed;
    m_pressed = nullptr;

    releaseTarget->SetControlState(
        releaseTarget == HitTestControl(x, y) ? ControlState::Hover : ControlState::Normal);

    auto upArgs = EventArgs::MakeMouse(EventType::MouseUp, x, y, button);
    upArgs.mouse.ctrl = IsCtrlHeld(m_input);
    upArgs.mouse.shift = IsShiftHeld(m_input);
    upArgs.mouse.alt = IsAltHeld(m_input);
    DispatchEvent(releaseTarget, upArgs);

    if (HitTestControl(x, y) == releaseTarget)
    {
        auto clickArgs = EventArgs::MakeMouse(EventType::Click, x, y, button);
        clickArgs.mouse.ctrl = IsCtrlHeld(m_input);
        clickArgs.mouse.shift = IsShiftHeld(m_input);
        clickArgs.mouse.alt = IsAltHeld(m_input);
        DispatchEvent(releaseTarget, clickArgs);
    }
}

void WindowHost::OnMouseLeave() noexcept
{
    if (m_hovered)
    {
        m_hovered->SetControlState(ControlState::Normal);
        DispatchEvent(m_hovered, EventArgs::MakeMouse(EventType::MouseLeave, 0, 0));
        m_hovered = nullptr;
    }
    m_pressed = nullptr;
}

void WindowHost::OnMouseWheel(float delta, float x, float y) noexcept
{
    if (!m_root) return;
    if (m_dialogs && m_dialogs->HandleMouseWheel(delta, x, y)) return;

    auto* target = HitTestControl(x, y);
    if (!target) target = m_hovered;
    if (!target) return;

    auto args = EventArgs::MakeMouse(EventType::MouseMove, x, y);
    args.mouse.wheelDelta = delta;
    args.mouse.ctrl = IsCtrlHeld(m_input);
    args.mouse.shift = IsShiftHeld(m_input);
    DispatchEvent(target, args);
}

void WindowHost::OnKeyDown(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept
{
    if (m_dialogs && m_dialogs->HandleKey(key, ctrl, shift, alt)) return;

    auto* focused = m_focusMgr.GetFocused();
    if (!focused) return;

    if (key == Input::KeyCode::Tab)
    {
        if (shift)
            m_focusMgr.FocusPrevious();
        else
            m_focusMgr.FocusNext();
        return;
    }

    KeyEventArgs args;
    args.key = key;
    args.ctrl = ctrl;
    args.shift = shift;
    args.alt = alt;

    EventArgs evt;
    evt.type = EventType::KeyDown;
    evt.key = args;
    DispatchEvent(focused, evt);
}

void WindowHost::OnTextInput(wchar_t ch) noexcept
{
    if (m_dialogs && m_dialogs->HandleText(ch)) return;

    auto* focused = m_focusMgr.GetFocused();
    if (!focused) return;

    EventArgs evt;
    evt.type = EventType::TextInput;
    evt.key = {Input::KeyCode::Unknown, ch};
    DispatchEvent(focused, evt);
}

} // namespace
