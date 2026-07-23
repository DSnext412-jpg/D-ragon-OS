#include <UI/Controls/PasswordBox.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

PasswordBox::PasswordBox() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultTextBox());
    SetAccessibilityRole(L"password");
}

void PasswordBox::SetPassword(std::wstring_view password) noexcept
{
    m_password = password;
    if (m_cursorPos > m_password.length())
        m_cursorPos = m_password.length();
    InvalidateVisual();
}

D2D1_RECT_F PasswordBox::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float fontSize = style ? style->fontSize : 14.0f;
    float paddingT = style ? style->paddingTop : 4.0f;
    float paddingB = style ? style->paddingBottom : 4.0f;

    float width = (std::min)(200.0f, available.right - available.left);
    float height = (std::max)(fontSize + paddingT + paddingB + 4.0f, 32.0f);
    height = (std::min)(height, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, width, height};
}

void PasswordBox::Render(UIRenderer& renderer) noexcept
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

    if (state == ElementState::Focused)
    {
        StyleStateColors focusColors = stateColors;
        focusColors.border = Theme::SemanticColor::Accent;
        renderer.DrawBorder(bounds, focusColors, style->borderThickness, cornerRadius, opacity);
    }
    else if (style->borderThickness > 0.0f)
    {
        renderer.DrawBorder(bounds, stateColors, style->borderThickness, cornerRadius, opacity);
    }

    float paddingL = style->paddingLeft;
    float paddingT = style->paddingTop;

    D2D1_RECT_F contentRect = {
        bounds.left + paddingL,
        bounds.top + paddingT,
        bounds.right - style->paddingRight,
        bounds.bottom - style->paddingBottom
    };

    if (m_password.empty() && !m_placeholder.empty())
    {
        StyleStateColors placeholderColors = stateColors;
        placeholderColors.foreground = Theme::SemanticColor::TextSecondary;
        renderer.DrawText(m_placeholder, contentRect, placeholderColors);
    }
    else if (!m_password.empty())
    {
        std::wstring bulletStr(m_password.length(), L'\u25CF');
        renderer.DrawText(bulletStr, contentRect, stateColors);
    }

    if (state == ElementState::Focused)
    {
        float fontSize = style->fontSize;
        float cursorX = bounds.left + paddingL +
            static_cast<float>(m_cursorPos) * 7.0f * (fontSize / 14.0f);
        float cursorY = bounds.top + paddingT;
        float cursorHeight = (std::max)(fontSize, bounds.bottom - bounds.top - paddingT - style->paddingBottom);

        D2D1_RECT_F cursorRect = {cursorX, cursorY, cursorX + 1.0f, cursorY + cursorHeight};
        StyleStateColors cursorColors;
        cursorColors.foreground = Theme::SemanticColor::TextPrimary;
        renderer.DrawBorder(cursorRect, cursorColors, 1.0f, 0.0f, opacity);

        renderer.DrawFocusIndicator(bounds);
    }
}

bool PasswordBox::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    switch (event.type)
    {
    case UIEventType::FocusGained:
        SetState(ElementState::Focused);
        InvalidateVisual();
        return true;

    case UIEventType::FocusLost:
        SetState(ElementState::Normal);
        InvalidateVisual();
        return true;

    case UIEventType::MouseEnter:
        if (GetState() != ElementState::Focused)
            SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        if (GetState() != ElementState::Focused)
            SetState(ElementState::Normal);
        return true;

    case UIEventType::MouseDown:
    {
        auto bounds = GetBounds();
        auto style = GetStyle();
        float paddingL = style ? style->paddingLeft : 0.0f;
        float fontSize = style ? style->fontSize : 14.0f;
        float clickX = event.x - bounds.left - paddingL;
        size_t pos = static_cast<size_t>((std::max)(0.0f, clickX) / (7.0f * (fontSize / 14.0f)));
        m_cursorPos = (std::min)(pos, m_password.length());
        InvalidateVisual();
        return true;
    }

    case UIEventType::Char:
        if (event.character >= 32 && m_password.length() < m_maxLength)
        {
            m_password.insert(m_cursorPos, 1, event.character);
            ++m_cursorPos;
            InvalidateVisual();
        }
        return true;

    case UIEventType::KeyDown:
    {
        switch (event.key)
        {
        case Input::KeyCode::Back:
            if (m_cursorPos > 0 && !m_password.empty())
            {
                m_password.erase(m_cursorPos - 1, 1);
                --m_cursorPos;
                InvalidateVisual();
            }
            return true;

        case Input::KeyCode::Delete:
            if (m_cursorPos < m_password.length())
            {
                m_password.erase(m_cursorPos, 1);
                InvalidateVisual();
            }
            return true;

        case Input::KeyCode::Left:
            if (m_cursorPos > 0) { --m_cursorPos; InvalidateVisual(); }
            return true;

        case Input::KeyCode::Right:
            if (m_cursorPos < m_password.length()) { ++m_cursorPos; InvalidateVisual(); }
            return true;

        case Input::KeyCode::Home:
            m_cursorPos = 0;
            InvalidateVisual();
            return true;

        case Input::KeyCode::End:
            m_cursorPos = m_password.length();
            InvalidateVisual();
            return true;

        case Input::KeyCode::Return:
            return true;

        default:
            return false;
        }
    }

    default:
        return false;
    }
}

} // namespace
