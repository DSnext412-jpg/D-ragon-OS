#include <DragonUI/Core/WindowHost.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

WindowHost::WindowHost(
    Graphics::Renderer& renderer,
    const Theme::ThemeManager& theme,
    Input::InputManager& input) noexcept
    : m_renderer(renderer)
    , m_theme(theme)
    , m_input(input)
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
}

void WindowHost::Update(float deltaTime) noexcept
{
    (void)deltaTime;
    if (!m_root) return;

    LayoutSlot viewport{0, 0, m_viewportW, m_viewportH};
    m_root->Measure(viewport);
    m_root->Arrange(viewport);
}

void WindowHost::Render() noexcept
{
    if (!m_root) return;

    RenderContext ctx(m_renderer, m_theme, m_dpiScale);
    m_root->Render(ctx);
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

void WindowHost::OnMouseMove(float x, float y) noexcept
{
    if (!m_root) return;
    UpdateHover(x, y);
}

void WindowHost::OnMouseDown(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_root) return;

    m_pressed = HitTestControl(x, y);

    if (m_pressed)
    {
        m_pressed->SetControlState(ControlState::Pressed);

        if (m_pressed->IsFocusable())
            m_focusMgr.SetFocus(m_pressed);

        DispatchEvent(m_pressed, EventArgs::MakeMouse(EventType::MouseDown, x, y, button));
    }
}

void WindowHost::OnMouseUp(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_root || !m_pressed) return;

    auto* releaseTarget = m_pressed;
    m_pressed = nullptr;

    releaseTarget->SetControlState(
        releaseTarget == HitTestControl(x, y) ? ControlState::Hover : ControlState::Normal);

    DispatchEvent(releaseTarget, EventArgs::MakeMouse(EventType::MouseUp, x, y, button));

    if (HitTestControl(x, y) == releaseTarget)
        DispatchEvent(releaseTarget, EventArgs::MakeMouse(EventType::Click, x, y, button));
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

void WindowHost::OnKeyDown(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept
{
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
    auto* focused = m_focusMgr.GetFocused();
    if (!focused) return;

    EventArgs evt;
    evt.type = EventType::TextInput;
    evt.key = {Input::KeyCode::Unknown, ch};
    DispatchEvent(focused, evt);
}

} // namespace
