#include <UI/Controls/RadioButton.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

// ── RadioButton ──────────────────────────────────────────────────────────

RadioButton::RadioButton() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultCheckBox());
    SetAccessibilityRole(L"radio");
}

RadioButton::RadioButton(std::wstring_view text) noexcept
    : m_text(text)
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultCheckBox());
    SetAccessibilityRole(L"radio");
}

void RadioButton::SetChecked(bool checked) noexcept
{
    if (m_checked == checked) return;

    if (checked && m_group)
    {
        m_group->Select(this);
        return;
    }

    m_checked = checked;
    if (m_onCheckedChanged)
        m_onCheckedChanged(m_checked);
    InvalidateVisual();
}

D2D1_RECT_F RadioButton::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float textWidth = static_cast<float>(m_text.length()) * 7.0f * (fontSize / 14.0f);
    float totalWidth = CircleSize + 6.0f + textWidth;
    float totalHeight = (std::max)(CircleSize, fontSize);

    totalWidth = (std::min)(totalWidth, available.right - available.left);
    totalHeight = (std::min)(totalHeight, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, totalWidth, totalHeight};
}

void RadioButton::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();

    float circleY = bounds.top + (bounds.bottom - bounds.top - CircleSize) * 0.5f;
    D2D1_RECT_F circleRect = {
        bounds.left,
        circleY,
        bounds.left + CircleSize,
        circleY + CircleSize
    };

    renderer.FillBackground(circleRect, stateColors, CircleSize * 0.5f, opacity);
    renderer.DrawBorder(circleRect, stateColors, style->borderThickness, CircleSize * 0.5f, opacity);

    if (m_checked)
    {
        float dotSize = CircleSize * 0.45f;
        float dotY = bounds.top + (bounds.bottom - bounds.top - dotSize) * 0.5f;
        D2D1_RECT_F dotRect = {
            bounds.left + (CircleSize - dotSize) * 0.5f,
            dotY,
            bounds.left + (CircleSize + dotSize) * 0.5f,
            dotY + dotSize
        };
        StyleStateColors dotColors;
        dotColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(dotRect, dotColors, dotSize * 0.5f, opacity);
    }

    float textX = bounds.left + CircleSize + 6.0f;
    D2D1_RECT_F textRect = {
        textX,
        bounds.top,
        bounds.right,
        bounds.bottom
    };

    if (!m_text.empty())
    {
        renderer.DrawText(m_text, textRect, stateColors);
    }

    if (state == ElementState::Focused)
    {
        renderer.DrawFocusIndicator(bounds);
    }
}

bool RadioButton::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        SetState(ElementState::Normal);
        return true;

    case UIEventType::MouseDown:
        SetState(ElementState::Pressed);
        return true;

    case UIEventType::MouseUp:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::Click:
        SetChecked(true);
        return true;

    case UIEventType::KeyDown:
        if (event.key == Input::KeyCode::Space)
        {
            SetState(ElementState::Pressed);
            return true;
        }
        return false;

    case UIEventType::KeyUp:
        if (event.key == Input::KeyCode::Space)
        {
            SetState(ElementState::Focused);
            SetChecked(true);
            return true;
        }
        return false;

    default:
        return false;
    }
}

// ── RadioButtonGroup ─────────────────────────────────────────────────────

void RadioButtonGroup::Remove(RadioButton* rb) noexcept
{
    auto it = std::find(m_buttons.begin(), m_buttons.end(), rb);
    if (it != m_buttons.end())
    {
        if (m_selected == rb)
            m_selected = nullptr;
        m_buttons.erase(it);
    }
}

void RadioButtonGroup::Select(RadioButton* rb) noexcept
{
    if (m_selected == rb) return;

    if (m_selected)
    {
        m_selected->m_checked = false;
        if (m_selected->m_onCheckedChanged)
            m_selected->m_onCheckedChanged(false);
        m_selected->InvalidateVisual();
    }

    m_selected = rb;

    if (m_selected)
    {
        m_selected->m_checked = true;
        if (m_selected->m_onCheckedChanged)
            m_selected->m_onCheckedChanged(true);
        m_selected->InvalidateVisual();
    }
}

} // namespace
