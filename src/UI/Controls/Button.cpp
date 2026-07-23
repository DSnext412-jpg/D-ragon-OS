#include <UI/Controls/Button.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

Button::Button() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultButton());
    SetAccessibilityRole(L"button");
}

Button::Button(std::wstring_view text) noexcept
    : m_text(text)
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultButton());
    SetAccessibilityRole(L"button");
}

D2D1_RECT_F Button::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float paddingL = style ? style->paddingLeft : 8.0f;
    float paddingR = style ? style->paddingRight : 8.0f;
    float paddingT = style ? style->paddingTop : 4.0f;
    float paddingB = style ? style->paddingBottom : 4.0f;

    float textWidth = static_cast<float>(m_text.length()) * 7.0f * (fontSize / 14.0f);
    float iconWidth = m_iconGlyph ? 20.0f : 0.0f;
    float totalWidth = paddingL + iconWidth + (iconWidth && !m_text.empty() ? 4.0f : 0.0f) + textWidth + paddingR;
    float totalHeight = fontSize + paddingT + paddingB;

    totalWidth = (std::max)(totalWidth, 32.0f);
    totalHeight = (std::max)(totalHeight, 32.0f);

    totalWidth = (std::min)(totalWidth, available.right - available.left);
    totalHeight = (std::min)(totalHeight, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, totalWidth, totalHeight};
}

void Button::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    SetBounds(finalRect);
}

void Button::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();
    float cornerRadius = style->cornerRadius;

    renderer.FillBackground(bounds, stateColors, cornerRadius, opacity);
    if (style->borderThickness > 0.0f)
    {
        renderer.DrawBorder(bounds, stateColors, style->borderThickness, cornerRadius, opacity);
    }

    float paddingL = style->paddingLeft;
    float paddingT = style->paddingTop;
    float paddingR = style->paddingRight;

    if (m_iconGlyph)
    {
        D2D1_RECT_F iconRect = {
            bounds.left + paddingL,
            bounds.top + paddingT,
            bounds.left + paddingL + 16.0f,
            bounds.bottom - paddingT
        };
        renderer.DrawIcon(iconRect, m_iconGlyph, stateColors, style->fontSize);
        paddingL += 20.0f;
    }

    D2D1_RECT_F textRect = {
        bounds.left + paddingL,
        bounds.top + paddingT,
        bounds.right - paddingR,
        bounds.bottom - style->paddingBottom
    };

    if (!m_text.empty())
    {
        renderer.DrawText(m_text, textRect, stateColors);
    }

    if (GetState() == ElementState::Focused)
    {
        renderer.DrawFocusIndicator(bounds);
    }
}

bool Button::OnEvent(const UIEvent& event) noexcept
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
    {
        SetState(ElementState::Hover);
        auto bounds = GetBounds();
        if (event.x >= bounds.left && event.x <= bounds.right &&
            event.y >= bounds.top && event.y <= bounds.bottom)
        {
            if (m_onClick) m_onClick();
        }
        return true;
    }

    case UIEventType::Click:
        if (m_onClick) m_onClick();
        return true;

    case UIEventType::KeyDown:
        if (event.key == Input::KeyCode::Space || event.key == Input::KeyCode::Return)
        {
            SetState(ElementState::Pressed);
            return true;
        }
        return false;

    case UIEventType::KeyUp:
        if (event.key == Input::KeyCode::Space || event.key == Input::KeyCode::Return)
        {
            SetState(ElementState::Focused);
            if (m_onClick) m_onClick();
            return true;
        }
        return false;

    default:
        return false;
    }
}

} // namespace
