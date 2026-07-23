#include <UI/Controls/ToggleSwitch.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

ToggleSwitch::ToggleSwitch() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultToggleSwitch());
    SetAccessibilityRole(L"switch");
}

ToggleSwitch::ToggleSwitch(std::wstring_view text) noexcept
    : m_text(text)
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultToggleSwitch());
    SetAccessibilityRole(L"switch");
}

void ToggleSwitch::SetToggled(bool toggled) noexcept
{
    if (m_toggled != toggled)
    {
        m_toggled = toggled;
        m_thumbOffset = m_toggled ? (TrackWidth - ThumbSize - 4.0f) : 0.0f;
        if (m_onToggled)
            m_onToggled(m_toggled);
        InvalidateVisual();
    }
}

D2D1_RECT_F ToggleSwitch::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float textWidth = static_cast<float>(m_text.length()) * 7.0f * (fontSize / 14.0f);
    float totalWidth = TrackWidth + 8.0f + textWidth;
    float totalHeight = (std::max)(TrackHeight, fontSize);

    totalWidth = (std::min)(totalWidth, available.right - available.left);
    totalHeight = (std::min)(totalHeight, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, totalWidth, totalHeight};
}

void ToggleSwitch::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();

    float trackY = bounds.top + (bounds.bottom - bounds.top - TrackHeight) * 0.5f;
    D2D1_RECT_F trackRect = {
        bounds.left,
        trackY,
        bounds.left + TrackWidth,
        trackY + TrackHeight
    };

    float trackRadius = TrackHeight * 0.5f;

    if (m_toggled)
    {
        StyleStateColors onColors = stateColors;
        onColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(trackRect, onColors, trackRadius, opacity);
        renderer.DrawBorder(trackRect, onColors, 1.0f, trackRadius, opacity);
    }
    else
    {
        renderer.FillBackground(trackRect, stateColors, trackRadius, opacity);
        renderer.DrawBorder(trackRect, stateColors, style->borderThickness, trackRadius, opacity);
    }

    float thumbX = bounds.left + 2.0f + m_thumbOffset;
    float thumbY = trackY + (TrackHeight - ThumbSize) * 0.5f;
    D2D1_RECT_F thumbRect = {
        thumbX,
        thumbY,
        thumbX + ThumbSize,
        thumbY + ThumbSize
    };

    StyleStateColors thumbColors;
    thumbColors.background = Theme::SemanticColor::TextPrimary;
    renderer.FillBackground(thumbRect, thumbColors, ThumbSize * 0.5f, opacity);

    float textX = bounds.left + TrackWidth + 8.0f;
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

bool ToggleSwitch::OnEvent(const UIEvent& event) noexcept
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
        SetToggled(!m_toggled);
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
            SetToggled(!m_toggled);
            return true;
        }
        return false;

    default:
        return false;
    }
}

} // namespace
