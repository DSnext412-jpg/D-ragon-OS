#include <DragonUI/Core/WindowHost.hpp>
#include <DragonUI/Dialogs/DialogManager.hpp>
#include <DragonUI/Controls/Button.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

WindowHost::WindowHost(
    Graphics::Renderer& renderer,
    Theme::ThemeManager& theme,
    Input::InputManager& input) noexcept
    : m_renderer(renderer)
    , m_theme(theme)
    , m_input(input)
    , m_dialogs(std::make_unique<DialogManager>())
{
    m_accessibility.Initialize(m_root.get(), &m_focusMgr, &m_theme);
    m_focusMgr.SetOnFocusChanged(
        [this](Control* focused) { OnFocusChanged(focused); });
}

WindowHost::~WindowHost() noexcept
{
    m_accessibility.Shutdown();
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

    m_accessibility.SetRoot(m_root.get());
}

void WindowHost::OnFocusChanged(Control* focused) noexcept
{
    m_accessibility.ReportFocus(focused);
    if (m_inspector)
    {
        m_inspector->SetFocusedElement(focused);
        m_inspector->Refresh();
    }
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

    // ── Developer shortcut: toggle the Accessibility Inspector ────────
    if (ctrl && shift && key == Input::KeyCode::A)
    {
        ToggleInspector();
        return;
    }

    auto* focused = m_focusMgr.GetFocused();

    // ── Tab order navigation ───────────────────────────────────────────
    if (key == Input::KeyCode::Tab)
    {
        if (shift)
            m_focusMgr.FocusPrevious();
        else
            m_focusMgr.FocusNext();
        return;
    }

    // ── Access keys (Alt + letter/digit) ───────────────────────────────
    if (alt)
    {
        wchar_t accessKey = 0;
        if (key >= Input::KeyCode::A && key <= Input::KeyCode::Z)
            accessKey = static_cast<wchar_t>(static_cast<uint32_t>(key) - static_cast<uint32_t>(Input::KeyCode::A) + L'A');
        else if (key >= Input::KeyCode::D0 && key <= Input::KeyCode::D9)
            accessKey = static_cast<wchar_t>(static_cast<uint32_t>(key) - static_cast<uint32_t>(Input::KeyCode::D0) + L'0');

        if (accessKey != 0)
        {
            if (Control* hit = m_focusMgr.FindByAccessKey(accessKey))
            {
                m_focusMgr.SetFocus(hit);
                return;
            }
        }
    }

    KeyEventArgs args;
    args.key = key;
    args.ctrl = ctrl;
    args.shift = shift;
    args.alt = alt;

    EventArgs evt;
    evt.type = EventType::KeyDown;
    evt.key = args;

    // Let the focused control process the key first (e.g. arrow keys move
    // the text caret inside a text box).
    bool handled = false;
    if (focused)
        handled = focused->OnEvent(evt);

    // ── Logical navigation fallback ────────────────────────────────────
    if (!handled)
    {
        switch (key)
        {
        case Input::KeyCode::Up:    m_focusMgr.MoveFocusDirection(FocusDirection::Up);    break;
        case Input::KeyCode::Down:  m_focusMgr.MoveFocusDirection(FocusDirection::Down);  break;
        case Input::KeyCode::Left:  m_focusMgr.MoveFocusDirection(FocusDirection::Left);  break;
        case Input::KeyCode::Right: m_focusMgr.MoveFocusDirection(FocusDirection::Right); break;
        case Input::KeyCode::Return:
            m_focusMgr.ActivateFocused();
            break;
        default:
            break;
        }
    }
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

// ── Accessibility Inspector (developer tool) ──────────────────────────────

void WindowHost::ToggleInspector() noexcept
{
    if (m_inspector)
    {
        // Remove the inspector view from the tree, then destroy it.
        auto* view = m_inspector->GetView();
        if (auto* container = dynamic_cast<Container*>(m_root.get()))
            container->RemoveChild(view);
        m_inspector.reset();
        m_accessibility.ReportStructuredChange();
        return;
    }

    m_inspector = std::make_unique<AccessibilityInspector>(m_accessibility);
    auto view = m_inspector->CreateView();
    if (!view)
    {
        m_inspector.reset();
        return;
    }

    auto* container = dynamic_cast<Container*>(m_root.get());
    if (!container)
    {
        m_inspector.reset();
        return;
    }

    container->AddChild(std::move(view));
    PlaceInspector();
    m_accessibility.ReportStructuredChange();
    m_accessibility.ReportFocus(m_focusMgr.GetFocused());
}

void WindowHost::PlaceInspector() noexcept
{
    auto* view = m_inspector ? m_inspector->GetView() : nullptr;
    if (!view)
        return;

    const float w = 360.0f;
    const float margin = 12.0f;
    const float h = (std::max)(240.0f, m_viewportH - margin * 2.0f);
    const float y = margin;
    const float x = m_viewportW - w - margin;
    view->SetBounds(LayoutSlot{ x, y, w, h });
}

} // namespace
