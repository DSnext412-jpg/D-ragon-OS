#include <DragonUI/Controls/RadioButton.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

// ── RadioGroup ────────────────────────────────────────────────────────

void RadioGroup::AddButton(UIRadioButton* button) noexcept
{
    if (!button) return;
    auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
    if (it == m_buttons.end())
        m_buttons.push_back(button);
}

void RadioGroup::RemoveButton(UIRadioButton* button) noexcept
{
    auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
    if (it != m_buttons.end())
    {
        if (m_selected == button)
            m_selected = nullptr;
        m_buttons.erase(it);
    }
}

void RadioGroup::Select(UIRadioButton* button) noexcept
{
    if (m_selected == button) return;

    if (m_selected)
        m_selected->SetChecked(false);

    m_selected = button;

    if (m_selected)
        m_selected->SetChecked(true);
}

void RadioGroup::ClearSelection() noexcept
{
    if (m_selected)
    {
        m_selected->SetChecked(false);
        m_selected = nullptr;
    }
}

// ── UIRadioButton ─────────────────────────────────────────────────────

UIRadioButton::UIRadioButton(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
}

UIRadioButton::~UIRadioButton() noexcept
{
    if (m_group)
        m_group->RemoveButton(this);
}

void UIRadioButton::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

void UIRadioButton::SetChecked(bool checked) noexcept
{
    if (m_checked == checked) return;
    m_checked = checked;
    InvalidateVisual();
    if (m_checked && m_onCheckedChanged)
        m_onCheckedChanged(*this);
}

void UIRadioButton::SetGroup(RadioGroup* group) noexcept
{
    if (m_group == group) return;
    if (m_group)
        m_group->RemoveButton(this);
    m_group = group;
    if (m_group)
        m_group->AddButton(this);
}

void UIRadioButton::NotifyGroup() noexcept
{
    if (m_group)
        m_group->Select(this);
    else
        SetChecked(true);
}

// ── Measure / Render ─────────────────────────────────────────────────

DesiredSize UIRadioButton::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f;
    float totalW = DotSize + Spacing + textW;
    return {std::min(totalW, available.width), std::max(DotSize + 4.0f, 26.0f)};
}

void UIRadioButton::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    bool isDisabled = GetControlState() == ControlState::Disabled;
    bool isHover = GetControlState() == ControlState::Hover;
    bool isFocused = GetControlState() == ControlState::Focused;

    float centerY = d2d.top + slot.height * 0.5f;

    // Outer circle
    D2D1_ELLIPSE outerEllipse = {
        d2d.left + DotSize * 0.5f,
        centerY,
        DotSize * 0.5f,
        DotSize * 0.5f,
    };

    auto outerBg = isDisabled ? Theme::SemanticColor::Disabled
        : isHover ? Theme::SemanticColor::Hover
        : Theme::SemanticColor::ControlFill;
    auto outerBorder = isDisabled ? Theme::SemanticColor::Disabled
        : isFocused ? Theme::SemanticColor::ControlAccentBorder
        : Theme::SemanticColor::ControlBorder;

    auto* rt = ctx.Renderer().GetRenderTarget();
    if (rt)
    {
        auto bgBrush = ctx.Renderer().GetBrush(ctx.Resolve(outerBg));
        if (bgBrush) rt->FillEllipse(&outerEllipse, bgBrush);

        auto borderBrush = ctx.Renderer().GetBrush(ctx.Resolve(outerBorder));
        if (borderBrush) rt->DrawEllipse(&outerEllipse, borderBrush, isFocused ? 2.0f : 1.0f);
    }

    // Inner dot (when checked)
    if (m_checked)
    {
        D2D1_ELLIPSE innerEllipse = {
            d2d.left + DotSize * 0.5f,
            centerY,
            InnerDotSize * 0.5f,
            InnerDotSize * 0.5f,
        };

        auto dotColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::ControlAccentFill;

        auto dotBrush = ctx.Renderer().GetBrush(ctx.Resolve(dotColor));
        if (dotBrush && rt) rt->FillEllipse(&innerEllipse, dotBrush);
    }

    // Focus rectangle
    if (isFocused && rt)
    {
        D2D1_RECT_F focusRect = {
            d2d.left - 2, centerY - DotSize * 0.5f - 2,
            d2d.left + DotSize + 2, centerY + DotSize * 0.5f + 2
        };
        ctx.DrawRectangle(focusRect, Theme::SemanticColor::ControlAccentBorder, 1.5f);
    }

    // Label text
    if (!m_text.empty())
    {
        auto textColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::TextPrimary;
        D2D1_RECT_F textRect = {
            d2d.left + DotSize + Spacing, d2d.top,
            d2d.right, d2d.bottom
        };
        ctx.DrawText(m_text, textRect, textColor);
    }
}

// ── Events ────────────────────────────────────────────────────────────

bool UIRadioButton::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::Click)
    {
        if (IsEnabled())
        {
            NotifyGroup();
            return true;
        }
    }
    return false;
}

bool UIRadioButton::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::KeyDown &&
        (args.key == Input::KeyCode::Space || args.key == Input::KeyCode::Return))
    {
        if (IsEnabled())
        {
            NotifyGroup();
            return true;
        }
    }
    return false;
}

} // namespace
