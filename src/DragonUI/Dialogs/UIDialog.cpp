#include <DragonUI/Dialogs/UIDialog.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

namespace DragonOS::DragonUI {

UIDialog::UIDialog(std::wstring_view title, bool modal) noexcept
    : m_title(title)
    , m_modal(modal)
{
    auto titleText = std::make_unique<UILabel>(m_title);
    titleText->SetVerticalAlignment(Alignment::Center);
    titleText->SetTextColor(Theme::SemanticColor::WindowTitle);
    m_titleText = titleText.get();
    AddChild(std::move(titleText));

    auto closeBtn = std::make_unique<UIButton>();
    closeBtn->SetIcon(L'\u2715');
    closeBtn->SetMinSize(22.0f, 22.0f);
    closeBtn->SetMaxSize(26.0f, 26.0f);
    closeBtn->SetCornerRadius(3.0f);
    closeBtn->SetOnClick([this](UIButton&) noexcept { Close(DialogResult::Cancel); });
    m_closeButton = closeBtn.get();
    AddChild(std::move(closeBtn));

    auto content = std::make_unique<UIStackPanel>(Orientation::Vertical);
    content->SetSpacing(10.0f);
    content->SetPadding({ClientPadding, 12.0f, ClientPadding, 4.0f});
    m_contentPanel = content.get();
    AddChild(std::move(content));

    auto buttons = std::make_unique<UIStackPanel>(Orientation::Horizontal);
    buttons->SetSpacing(8.0f);
    buttons->SetHorizontalAlignment(Alignment::End);
    buttons->SetVerticalAlignment(Alignment::Center);
    m_buttonPanel = buttons.get();
    AddChild(std::move(buttons));

    m_focusMgr.RegisterRoot(this);
}

// ── Identity / state ────────────────────────────────────────────────────

void UIDialog::SetTitle(std::wstring_view title) noexcept
{
    m_title = title;
    if (m_titleText)
        m_titleText->SetText(title);
}

// ── Size / position ─────────────────────────────────────────────────────

void UIDialog::SetSize(float width, float height) noexcept
{
    m_width = (std::max)(width, m_minW);
    m_height = (std::max)(height, m_minH);
    m_bounds.width = m_width;
    m_bounds.height = m_height;
    InvalidateLayout();
}

void UIDialog::SetMinSize(float width, float height) noexcept
{
    m_minW = width;
    m_minH = height;
}

void UIDialog::SetPosition(float x, float y) noexcept
{
    m_bounds.x = x;
    m_bounds.y = y;
    InvalidateLayout();
}

void UIDialog::CenterIn(float viewportWidth, float viewportHeight) noexcept
{
    m_viewportW = viewportWidth;
    m_viewportH = viewportHeight;
    SetPosition((viewportWidth - m_width) * 0.5f, (viewportHeight - m_height) * 0.5f);
}

// ── Content ─────────────────────────────────────────────────────────────

void UIDialog::AddContent(std::unique_ptr<Element> child) noexcept
{
    if (m_contentPanel && child)
        m_contentPanel->AddChild(std::move(child));
}

UIButton* UIDialog::AddButton(std::wstring_view text, DialogResult result) noexcept
{
    return AddButton(text, [this, result](UIButton&) noexcept { Close(result); });
}

UIButton* UIDialog::AddButton(std::wstring_view text, UIButton::ClickCallback onClick) noexcept
{
    if (!m_buttonPanel)
        return nullptr;

    auto btn = std::make_unique<UIButton>(text);
    UIButton* raw = btn.get();
    btn->SetOnClick(std::move(onClick));
    m_buttonPanel->AddChild(std::move(btn));
    return raw;
}

void UIDialog::SetDefaultButton(UIButton* button) noexcept
{
    m_defaultButton = button;
    if (m_defaultButton && m_open)
        m_focusMgr.SetFocus(m_defaultButton);
}

void UIDialog::SetCancelButton(UIButton* button) noexcept
{
    m_cancelButton = button;
}

// ── Lifecycle ───────────────────────────────────────────────────────────

void UIDialog::Show() noexcept
{
    if (m_open)
        return;
    m_open = true;
    m_result = DialogResult::None;
    m_dragging = false;
    m_resizeEdge = ResizeEdge::None;
    m_pressed = nullptr;
    m_lastClickTarget = nullptr;
    InvalidateLayout();
    FocusDefault();
}

void UIDialog::Close(DialogResult result) noexcept
{
    if (!m_open)
        return;

    m_open = false;
    m_result = result;
    m_dragging = false;
    m_resizeEdge = ResizeEdge::None;
    m_pressed = nullptr;

    if (m_hovered)
    {
        if (auto* ctrl = dynamic_cast<Control*>(m_hovered))
            ctrl->SetControlState(ControlState::Normal);
        m_hovered = nullptr;
    }

    m_focusMgr.SetFocus(nullptr);

    if (m_onClosed)
        m_onClosed(*this, result);
}

void UIDialog::FocusDefault() noexcept
{
    if (m_defaultButton)
        m_focusMgr.SetFocus(m_defaultButton);
    else
        m_focusMgr.FocusFirst();
}

// ── Layout ──────────────────────────────────────────────────────────────

DesiredSize UIDialog::MeasureOverride(const LayoutSlot& available) noexcept
{
    (void)available;
    return {m_width, m_height};
}

void UIDialog::Measure(const LayoutSlot& available) noexcept
{
    (void)available;
    if (m_visibility != Visibility::Visible)
    {
        m_desiredSize = {};
        return;
    }

    const float b = BorderThickness;
    LayoutSlot titleSlot{0, 0, m_width - 2.0f * b, TitleBarHeight};
    LayoutSlot closeSlot{0, 0, 34.0f, TitleBarHeight};
    LayoutSlot contentSlot{0, 0, m_width - 2.0f * b,
                           m_height - 2.0f * b - TitleBarHeight - 36.0f};
    LayoutSlot btnSlot{0, 0, m_width - 2.0f * b - 2.0f * ClientPadding, 32.0f};

    if (m_titleText) m_titleText->Measure(titleSlot);
    if (m_closeButton) m_closeButton->Measure(closeSlot);
    if (m_contentPanel) m_contentPanel->Measure(contentSlot);
    if (m_buttonPanel) m_buttonPanel->Measure(btnSlot);

    m_desiredSize = {m_width, m_height};
    m_layoutDirty = false;
}

void UIDialog::Arrange(const LayoutSlot& finalSlot) noexcept
{
    (void)finalSlot;
    auto content = m_bounds.Inset(m_padding);
    ArrangeOverride(content);
    ArrangeChildren(content);
    m_layoutDirty = false;
}

void UIDialog::ArrangeOverride(const LayoutSlot& /*finalSlot*/) noexcept
{
    const float b = BorderThickness;
    const float innerLeft = m_bounds.x + b;
    const float innerRight = m_bounds.x + m_width - b;
    const float innerBottom = m_bounds.y + m_height - b;

    if (m_titleText)
    {
        m_titleText->Arrange({innerLeft + 12.0f,
                              m_bounds.y + (TitleBarHeight - 20.0f) * 0.5f,
                              innerRight - innerLeft - 60.0f, 20.0f});
    }

    if (m_closeButton)
    {
        m_closeButton->Arrange({innerRight - 26.0f - 5.0f,
                                m_bounds.y + (TitleBarHeight - 26.0f) * 0.5f,
                                26.0f, 26.0f});
    }

    const float btnRowH = 36.0f;
    const float btnRowTop = innerBottom - btnRowH;

    if (m_buttonPanel)
    {
        m_buttonPanel->Arrange({innerLeft + ClientPadding, btnRowTop,
                                innerRight - innerLeft - 2.0f * ClientPadding, 32.0f});
    }

    if (m_contentPanel)
    {
        m_contentPanel->Arrange({innerLeft, m_bounds.y + TitleBarHeight,
                                 innerRight - innerLeft, btnRowTop - (m_bounds.y + TitleBarHeight)});
    }
}

void UIDialog::ArrangeChildren(const LayoutSlot& /*finalSlot*/) noexcept
{
}

// ── Render ──────────────────────────────────────────────────────────────

void UIDialog::Render(RenderContext& ctx) noexcept
{
    if (!m_open || m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    auto d2d = static_cast<D2D1_RECT_F>(m_bounds);

    ctx.FillRectangle(d2d, Theme::SemanticColor::WindowBackground);

    D2D1_RECT_F titleBar = {d2d.left, d2d.top, d2d.right, d2d.top + TitleBarHeight};
    ctx.FillRectangle(titleBar, Theme::SemanticColor::WindowTitleBar);

    Container::Render(ctx);

    ctx.DrawRectangle(d2d, Theme::SemanticColor::WindowBorder, 1.0f);
}

// ── Hit testing helpers ─────────────────────────────────────────────────

bool UIDialog::IsInTitleBarRegion(float x, float y) const noexcept
{
    return x >= m_bounds.x && x <= m_bounds.x + m_width &&
           y >= m_bounds.y && y <= m_bounds.y + TitleBarHeight;
}

UIDialog::ResizeEdge UIDialog::HitTestResizeEdge(float x, float y) const noexcept
{
    if (!m_resizable)
        return ResizeEdge::None;

    const float g = 6.0f;
    const float left = m_bounds.x;
    const float right = m_bounds.x + m_width;
    const float top = m_bounds.y;
    const float bottom = m_bounds.y + m_height;

    const bool nearLeft = x >= left && x <= left + g;
    const bool nearRight = x <= right && x >= right - g;
    const bool nearTop = y >= top && y <= top + g;
    const bool nearBottom = y <= bottom && y >= bottom - g;

    if (nearLeft && nearTop) return ResizeEdge::TopLeft;
    if (nearRight && nearTop) return ResizeEdge::TopRight;
    if (nearLeft && nearBottom) return ResizeEdge::BottomLeft;
    if (nearRight && nearBottom) return ResizeEdge::BottomRight;
    if (nearLeft) return ResizeEdge::Left;
    if (nearRight) return ResizeEdge::Right;
    if (nearTop) return ResizeEdge::Top;
    if (nearBottom) return ResizeEdge::Bottom;
    return ResizeEdge::None;
}

void UIDialog::DispatchTo(Control* target, const EventArgs& args) noexcept
{
    if (target)
        (void)target->OnEvent(args);
}

void UIDialog::SetHover(float x, float y) noexcept
{
    Element* newHover = m_bounds.Contains(x, y) ? HitTest(x, y) : nullptr;

    if (newHover == m_hovered)
    {
        if (m_hovered)
        {
            if (auto* ctrl = dynamic_cast<Control*>(m_hovered))
            {
                auto args = EventArgs::MakeMouse(EventType::MouseMove, x, y);
                DispatchTo(ctrl, args);
            }
        }
        return;
    }

    if (m_hovered)
    {
        if (auto* ctrl = dynamic_cast<Control*>(m_hovered))
        {
            ctrl->SetControlState(ControlState::Normal);
            DispatchTo(ctrl, EventArgs::MakeMouse(EventType::MouseLeave, x, y));
        }
        m_hovered = nullptr;
    }

    m_hovered = newHover;

    if (m_hovered)
    {
        if (auto* ctrl = dynamic_cast<Control*>(m_hovered))
        {
            ctrl->SetControlState(ControlState::Hover);
            DispatchTo(ctrl, EventArgs::MakeMouse(EventType::MouseEnter, x, y));
        }
    }
}

// ── Input routing ───────────────────────────────────────────────────────

bool UIDialog::HandleMouseMove(float x, float y) noexcept
{
    if (!m_open)
        return false;

    if (m_dragging)
    {
        const float nx = (std::clamp)(x - m_dragOffsetX, 0.0f, (std::max)(0.0f, m_viewportW - m_width));
        const float ny = (std::clamp)(y - m_dragOffsetY, 0.0f, (std::max)(0.0f, m_viewportH - m_height));
        SetPosition(nx, ny);
        return true;
    }

    if (m_resizeEdge != ResizeEdge::None)
    {
        const float dx = x - m_resizeStartX;
        const float dy = y - m_resizeStartY;
        float newW = m_resizeStartW;
        float newH = m_resizeStartH;
        float newX = m_resizeStartXpos;
        float newY = m_resizeStartYpos;

        const bool left = m_resizeEdge == ResizeEdge::Left ||
                          m_resizeEdge == ResizeEdge::TopLeft ||
                          m_resizeEdge == ResizeEdge::BottomLeft;
        const bool right = m_resizeEdge == ResizeEdge::Right ||
                           m_resizeEdge == ResizeEdge::TopRight ||
                           m_resizeEdge == ResizeEdge::BottomRight;
        const bool top = m_resizeEdge == ResizeEdge::Top ||
                         m_resizeEdge == ResizeEdge::TopLeft ||
                         m_resizeEdge == ResizeEdge::TopRight;
        const bool bottom = m_resizeEdge == ResizeEdge::Bottom ||
                            m_resizeEdge == ResizeEdge::BottomLeft ||
                            m_resizeEdge == ResizeEdge::BottomRight;

        if (right) newW = m_resizeStartW + dx;
        if (bottom) newH = m_resizeStartH + dy;
        if (left) newW = m_resizeStartW - dx;
        if (top) newH = m_resizeStartH - dy;

        newW = (std::clamp)(newW, m_minW, m_viewportW);
        newH = (std::clamp)(newH, m_minH, m_viewportH);

        // Opposite edges stay fixed.
        if (left)
            newX = (m_resizeStartXpos + m_resizeStartW) - newW;
        if (top)
            newY = (m_resizeStartYpos + m_resizeStartH) - newH;

        SetPosition(newX, newY);
        SetSize(newW, newH);
        return true;
    }

    if (!m_bounds.Contains(x, y))
    {
        SetHover(x, y);
        return false;
    }

    SetHover(x, y);
    return true;
}

bool UIDialog::HandleMouseDown(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_open)
        return false;

    if (button == Input::MouseButton::Left)
    {
        ResizeEdge edge = HitTestResizeEdge(x, y);
        if (edge != ResizeEdge::None)
        {
            m_resizeEdge = edge;
            m_resizeStartX = x;
            m_resizeStartY = y;
            m_resizeStartW = m_width;
            m_resizeStartH = m_height;
            m_resizeStartXpos = m_bounds.x;
            m_resizeStartYpos = m_bounds.y;
            return true;
        }

        if (m_canDrag && IsInTitleBarRegion(x, y) &&
            !(m_closeButton && m_closeButton->GetBounds().Contains(x, y)))
        {
            m_dragging = true;
            m_dragOffsetX = x - m_bounds.x;
            m_dragOffsetY = y - m_bounds.y;
            return true;
        }
    }

    if (!m_bounds.Contains(x, y))
        return false;

    Element* target = HitTest(x, y);
    m_pressed = target;

    if (auto* ctrl = dynamic_cast<Control*>(target))
    {
        if (ctrl->IsFocusable())
            m_focusMgr.SetFocus(ctrl);
        ctrl->SetControlState(ControlState::Pressed);

        auto args = EventArgs::MakeMouse(EventType::MouseDown, x, y, button);
        DispatchTo(ctrl, args);
    }

    return true;
}

bool UIDialog::HandleMouseUp(float x, float y, Input::MouseButton button) noexcept
{
    if (!m_open)
        return false;

    if (m_dragging)
    {
        m_dragging = false;
        return true;
    }

    if (m_resizeEdge != ResizeEdge::None)
    {
        m_resizeEdge = ResizeEdge::None;
        return true;
    }

    if (!m_pressed)
        return false;

    auto* releaseTarget = m_pressed;
    m_pressed = nullptr;

    if (auto* ctrl = dynamic_cast<Control*>(releaseTarget))
    {
        ctrl->SetControlState(
            ctrl == HitTest(x, y) ? ControlState::Hover : ControlState::Normal);

        auto upArgs = EventArgs::MakeMouse(EventType::MouseUp, x, y, button);
        DispatchTo(ctrl, upArgs);

        if (HitTest(x, y) == releaseTarget)
        {
            auto clickArgs = EventArgs::MakeMouse(EventType::Click, x, y, button);
            DispatchTo(ctrl, clickArgs);

            const auto now = std::chrono::steady_clock::now();
            const double elapsed = std::chrono::duration<double>(now - m_lastClickTime).count();
            const bool sameTarget = releaseTarget == m_lastClickTarget;
            const bool nearClick = std::abs(x - m_lastClickX) < 4.0f && std::abs(y - m_lastClickY) < 4.0f;

            if (sameTarget && nearClick && elapsed < 0.35)
            {
                auto dblArgs = EventArgs::MakeMouse(EventType::DoubleClick, x, y, button, 2);
                DispatchTo(ctrl, dblArgs);
                m_lastClickTarget = nullptr;
            }
            else
            {
                m_lastClickTarget = releaseTarget;
            }
            m_lastClickTime = now;
            m_lastClickX = x;
            m_lastClickY = y;
        }
    }

    return true;
}

bool UIDialog::HandleMouseWheel(float delta, float x, float y) noexcept
{
    if (!m_open || !m_bounds.Contains(x, y))
        return false;

    Element* target = HitTest(x, y);
    if (!target)
        return false;

    if (auto* ctrl = dynamic_cast<Control*>(target))
    {
        auto args = EventArgs::MakeMouse(EventType::MouseMove, x, y);
        args.mouse.wheelDelta = delta;
        DispatchTo(ctrl, args);
    }

    return true;
}

bool UIDialog::HandleKey(Input::KeyCode key, bool ctrl, bool shift, bool alt) noexcept
{
    if (!m_open)
        return false;

    if (key == Input::KeyCode::Tab)
    {
        if (shift)
            m_focusMgr.FocusPrevious();
        else
            m_focusMgr.FocusNext();
        return true;
    }

    if (key == Input::KeyCode::Return || key == Input::KeyCode::Enter)
    {
        if (m_defaultButton)
        {
            EventArgs evt;
            evt.type = EventType::KeyDown;
            evt.key = {key, {}, false, ctrl, shift, alt};
            DispatchTo(m_defaultButton, evt);
            return true;
        }
    }

    if (key == Input::KeyCode::Escape)
    {
        if (m_cancelButton)
        {
            Close(DialogResult::Cancel);
            return true;
        }
    }

    auto* focused = m_focusMgr.GetFocused();
    if (!focused)
        return true;

    EventArgs evt;
    evt.type = EventType::KeyDown;
    evt.key = {key, {}, false, ctrl, shift, alt};
    DispatchTo(focused, evt);
    return true;
}

bool UIDialog::HandleText(wchar_t ch) noexcept
{
    if (!m_open)
        return false;

    auto* focused = m_focusMgr.GetFocused();
    if (!focused)
        return false;

    EventArgs evt;
    evt.type = EventType::TextInput;
    evt.key = {Input::KeyCode::Unknown, ch};
    DispatchTo(focused, evt);
    return true;
}

} // namespace DragonOS::DragonUI
