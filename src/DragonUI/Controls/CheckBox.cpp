#include <DragonUI/Controls/CheckBox.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UICheckBox::UICheckBox(std::wstring_view text) noexcept
    : m_text(text)
{
    m_focusable = true;
}

void UICheckBox::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateLayout();
}

void UICheckBox::SetChecked(bool checked) noexcept
{
    SetCheckState(checked ? CheckState::Checked : CheckState::Unchecked);
}

void UICheckBox::SetCheckState(CheckState state) noexcept
{
    if (m_checkState == state) return;
    m_checkState = state;
    InvalidateVisual();
    if (m_onCheckedChanged) m_onCheckedChanged(*this, m_checkState);
}

void UICheckBox::Toggle() noexcept
{
    switch (m_checkState)
    {
    case CheckState::Unchecked:
        SetCheckState(CheckState::Checked);
        break;
    case CheckState::Checked:
        SetCheckState(m_onCheckedChanged ? CheckState::Unchecked : CheckState::Unchecked);
        break;
    case CheckState::Indeterminate:
        SetCheckState(CheckState::Checked);
        break;
    }
}

// ── Measure / Render ─────────────────────────────────────────────────

DesiredSize UICheckBox::MeasureOverride(const LayoutSlot& available) noexcept
{
    float textW = static_cast<float>(m_text.size()) * 8.0f;
    float totalW = BoxSize + Spacing + textW;
    return {std::min(totalW, available.width), std::max(BoxSize + 4.0f, 26.0f)};
}

void UICheckBox::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    bool isDisabled = GetControlState() == ControlState::Disabled;
    bool isHover = GetControlState() == ControlState::Hover;
    bool isFocused = GetControlState() == ControlState::Focused;

    float centerY = d2d.top + slot.height * 0.5f;

    // Check box rect
    D2D1_RECT_F boxRect = {
        d2d.left, centerY - BoxSize * 0.5f,
        d2d.left + BoxSize, centerY + BoxSize * 0.5f
    };

    // Box background
    auto boxBg = isDisabled ? Theme::SemanticColor::Disabled
        : isHover ? Theme::SemanticColor::Hover
        : Theme::SemanticColor::ControlFill;
    ctx.FillRoundedRect(boxRect, boxBg, 3.0f, 3.0f);

    // Box border
    auto boxBorder = isDisabled ? Theme::SemanticColor::Disabled
        : isFocused ? Theme::SemanticColor::ControlAccentBorder
        : Theme::SemanticColor::ControlBorder;
    ctx.DrawRoundedRect(boxRect, boxBorder, 3.0f, 3.0f, isFocused ? 2.0f : 1.0f);

    // Check mark / indeterminate dash
    if (m_checkState == CheckState::Checked)
    {
        auto checkColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::ControlAccentFill;

        // Draw a checkmark using two lines
        float s = BoxSize * 0.25f;
        float cx = boxRect.left + BoxSize * 0.5f;
        float cy = boxRect.top + BoxSize * 0.5f;

        ctx.Renderer().DrawLine(
            D2D1::Point2F(cx - s, cy),
            D2D1::Point2F(cx - s * 0.3f, cy + s * 0.7f),
            ctx.Resolve(checkColor), 2.0f);
        ctx.Renderer().DrawLine(
            D2D1::Point2F(cx - s * 0.3f, cy + s * 0.7f),
            D2D1::Point2F(cx + s, cy - s * 0.5f),
            ctx.Resolve(checkColor), 2.0f);
    }
    else if (m_checkState == CheckState::Indeterminate)
    {
        auto dashColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::ControlAccentFill;
        float dashPad = BoxSize * 0.25f;
        D2D1_RECT_F dashRect = {
            boxRect.left + dashPad,
            boxRect.top + BoxSize * 0.35f,
            boxRect.right - dashPad,
            boxRect.bottom - BoxSize * 0.35f
        };
        ctx.FillRectangle(dashRect, dashColor);
    }

    // Label text
    if (!m_text.empty())
    {
        auto textColor = isDisabled ? Theme::SemanticColor::Disabled
            : Theme::SemanticColor::TextPrimary;
        D2D1_RECT_F textRect = {
            boxRect.right + Spacing, d2d.top,
            d2d.right, d2d.bottom
        };
        ctx.DrawText(m_text, textRect, textColor);
    }

    // Focus rectangle
    if (isFocused)
    {
        D2D1_RECT_F focusRect = {
            boxRect.left - 2, boxRect.top - 2,
            boxRect.right + 2, boxRect.bottom + 2
        };
        ctx.DrawRectangle(focusRect, Theme::SemanticColor::ControlAccentBorder, 1.5f);
    }
}

// ── Events ────────────────────────────────────────────────────────────

bool UICheckBox::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::Click)
    {
        if (IsEnabled())
        {
            Toggle();
            return true;
        }
    }
    return false;
}

bool UICheckBox::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type == EventType::KeyDown &&
        (args.key == Input::KeyCode::Space || args.key == Input::KeyCode::Return))
    {
        if (IsEnabled())
        {
            Toggle();
            return true;
        }
    }
    return false;
}

} // namespace
