#include <UI/Controls/Slider.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>
#include <cmath>
#include <format>

namespace DragonOS::UI {

Slider::Slider() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultSlider());
    SetAccessibilityRole(L"slider");
}

void Slider::SetValue(float value) noexcept
{
    value = ClampValue(value);
    if (m_step > 0.0f)
        value = std::round(value / m_step) * m_step;
    value = ClampValue(value);

    if (m_value != value)
    {
        m_value = value;
        if (m_onValueChanged)
            m_onValueChanged(m_value);
        InvalidateVisual();
    }
}

float Slider::ClampValue(float val) const noexcept
{
    return (std::max)(m_min, (std::min)(m_max, val));
}

float Slider::ValueToPosition() const noexcept
{
    auto bounds = GetBounds();
    float trackLeft = bounds.left + ThumbRadius + 2.0f;
    float trackRight = bounds.right - ThumbRadius - 2.0f;
    float range = m_max - m_min;
    if (range <= 0.0f) return trackLeft;
    float t = (m_value - m_min) / range;
    return trackLeft + t * (trackRight - trackLeft);
}

void Slider::PositionToValue(float x) noexcept
{
    auto bounds = GetBounds();
    float trackLeft = bounds.left + ThumbRadius + 2.0f;
    float trackRight = bounds.right - ThumbRadius - 2.0f;
    if (trackRight <= trackLeft) return;

    float t = (x - trackLeft) / (trackRight - trackLeft);
    t = (std::max)(0.0f, (std::min)(1.0f, t));
    float val = m_min + t * (m_max - m_min);

    if (m_step > 0.0f)
        val = std::round(val / m_step) * m_step;
    val = ClampValue(val);

    if (m_value != val)
    {
        m_value = val;
        if (m_onValueChanged)
            m_onValueChanged(m_value);
        InvalidateVisual();
    }
}

D2D1_RECT_F Slider::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float paddingT = style ? style->paddingTop : 4.0f;
    float paddingB = style ? style->paddingBottom : 4.0f;

    float width = (std::max)(MinWidth, available.right - available.left);
    float height = ThumbRadius * 2.0f + paddingT + paddingB;
    height = (std::min)(height, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, width, height};
}

void Slider::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();

    float trackY = bounds.top + (bounds.bottom - bounds.top) * 0.5f - TrackHeight * 0.5f;
    float trackLeft = bounds.left + ThumbRadius + 2.0f;
    float trackRight = bounds.right - ThumbRadius - 2.0f;

    D2D1_RECT_F trackBgRect = {
        trackLeft,
        trackY,
        trackRight,
        trackY + TrackHeight
    };
    renderer.FillBackground(trackBgRect, stateColors, TrackHeight * 0.5f, opacity);

    float thumbPos = ValueToPosition();
    D2D1_RECT_F fillRect = {
        trackLeft,
        trackY,
        (std::max)(trackLeft, thumbPos),
        trackY + TrackHeight
    };
    if (fillRect.right > fillRect.left)
    {
        StyleStateColors fillColors = stateColors;
        fillColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(fillRect, fillColors, TrackHeight * 0.5f, opacity);
    }

    D2D1_RECT_F thumbRect = {
        thumbPos - ThumbRadius,
        bounds.top + (bounds.bottom - bounds.top) * 0.5f - ThumbRadius,
        thumbPos + ThumbRadius,
        bounds.top + (bounds.bottom - bounds.top) * 0.5f + ThumbRadius
    };
    StyleStateColors thumbColors;
    thumbColors.background = state == ElementState::Pressed
        ? Theme::SemanticColor::AccentPressed
        : Theme::SemanticColor::Accent;
    thumbColors.border = Theme::SemanticColor::WindowBorder;
    renderer.FillBackground(thumbRect, thumbColors, ThumbRadius, opacity);
    renderer.DrawBorder(thumbRect, thumbColors, 1.0f, ThumbRadius, opacity);

    if (state == ElementState::Focused)
    {
        std::wstring valStr = std::format(L"{:.0f}", m_value);
        D2D1_RECT_F valRect = {
            thumbPos - 20.0f,
            bounds.top,
            thumbPos + 20.0f,
            bounds.top + style->fontSize + 2.0f
        };
        renderer.DrawText(valStr, valRect, stateColors);
        renderer.DrawFocusIndicator(bounds);
    }
}

bool Slider::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        if (!m_isDragging)
            SetState(ElementState::Normal);
        return true;

    case UIEventType::MouseDown:
        m_isDragging = true;
        SetState(ElementState::Pressed);
        PositionToValue(event.x);
        return true;

    case UIEventType::MouseMove:
        if (m_isDragging)
        {
            PositionToValue(event.x);
            return true;
        }
        return false;

    case UIEventType::MouseUp:
        if (m_isDragging)
        {
            m_isDragging = false;
            SetState(ElementState::Hover);
            PositionToValue(event.x);
            return true;
        }
        return false;

    case UIEventType::KeyDown:
        switch (event.key)
        {
        case Input::KeyCode::Left:
        case Input::KeyCode::Down:
            SetValue(m_value - m_step);
            return true;
        case Input::KeyCode::Right:
        case Input::KeyCode::Up:
            SetValue(m_value + m_step);
            return true;
        case Input::KeyCode::Home:
            SetValue(m_min);
            return true;
        case Input::KeyCode::End:
            SetValue(m_max);
            return true;
        default:
            return false;
        }

    case UIEventType::FocusGained:
        SetState(ElementState::Focused);
        return true;

    case UIEventType::FocusLost:
        SetState(ElementState::Normal);
        return true;

    default:
        return false;
    }
}

} // namespace
