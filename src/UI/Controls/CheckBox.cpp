#include <UI/Controls/CheckBox.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

CheckBox::CheckBox() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultCheckBox());
    SetAccessibilityRole(L"checkbox");
}

CheckBox::CheckBox(std::wstring_view text) noexcept
    : m_text(text)
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultCheckBox());
    SetAccessibilityRole(L"checkbox");
}

void CheckBox::SetChecked(bool checked) noexcept
{
    if (m_checked != checked)
    {
        m_checked = checked;
        if (m_onCheckedChanged)
            m_onCheckedChanged(m_checked);
        InvalidateVisual();
    }
}

D2D1_RECT_F CheckBox::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float textWidth = static_cast<float>(m_text.length()) * 7.0f * (fontSize / 14.0f);
    float totalWidth = BoxSize + 6.0f + textWidth;
    float totalHeight = (std::max)(BoxSize, fontSize);

    totalWidth = (std::min)(totalWidth, available.right - available.left);
    totalHeight = (std::min)(totalHeight, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, totalWidth, totalHeight};
}

void CheckBox::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();

    float boxY = bounds.top + (bounds.bottom - bounds.top - BoxSize) * 0.5f;
    D2D1_RECT_F boxRect = {
        bounds.left,
        boxY,
        bounds.left + BoxSize,
        boxY + BoxSize
    };

    if (m_checked)
    {
        StyleStateColors fillColors = stateColors;
        fillColors.background = Theme::SemanticColor::Accent;
        fillColors.border = Theme::SemanticColor::Accent;
        renderer.FillBackground(boxRect, fillColors, 3.0f, opacity);
        renderer.DrawBorder(boxRect, fillColors, 1.0f, 3.0f, opacity);

        D2D1_RECT_F checkRect = {
            boxRect.left + 2.0f,
            boxRect.top + 2.0f,
            boxRect.right - 2.0f,
            boxRect.bottom - 2.0f
        };
        renderer.DrawIcon(checkRect, static_cast<wchar_t>(0x2713), fillColors, BoxSize - 4.0f);
    }
    else
    {
        renderer.FillBackground(boxRect, stateColors, 3.0f, opacity);
        renderer.DrawBorder(boxRect, stateColors, style->borderThickness, 3.0f, opacity);
    }

    float textX = bounds.left + BoxSize + 6.0f;
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

bool CheckBox::OnEvent(const UIEvent& event) noexcept
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
        SetChecked(!m_checked);
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
            SetChecked(!m_checked);
            return true;
        }
        return false;

    default:
        return false;
    }
}

} // namespace
