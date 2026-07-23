#include <UI/Controls/TextBox.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <UI/Core/FocusManager.hpp>
#include <algorithm>
#include <chrono>

namespace DragonOS::UI {

TextBox::TextBox() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultTextBox());
    SetAccessibilityRole(L"textbox");
}

TextBox::TextBox(std::wstring_view placeholder) noexcept
    : m_placeholder(placeholder)
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultTextBox());
    SetAccessibilityRole(L"textbox");
}

void TextBox::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    if (m_cursorPos > m_text.length())
        m_cursorPos = m_text.length();
    if (m_onTextChanged)
        m_onTextChanged(m_text);
    InvalidateVisual();
}

void TextBox::SetCursorPosition(size_t pos) noexcept
{
    m_cursorPos = (std::min)(pos, m_text.length());
    InvalidateVisual();
}

D2D1_RECT_F TextBox::MeasureOverride(const D2D1_RECT_F& available) noexcept
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

void TextBox::Render(UIRenderer& renderer) noexcept
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
        bounds.left + paddingL + m_scrollOffset,
        bounds.top + paddingT,
        bounds.right - style->paddingRight + m_scrollOffset,
        bounds.bottom - style->paddingBottom
    };

    if (m_text.empty() && !m_placeholder.empty())
    {
        StyleStateColors placeholderColors = stateColors;
        placeholderColors.foreground = Theme::SemanticColor::TextSecondary;
        renderer.DrawText(m_placeholder, contentRect, placeholderColors);
    }
    else if (!m_text.empty())
    {
        renderer.DrawText(m_text, contentRect, stateColors);
    }

    if (state == ElementState::Focused)
    {
        float fontSize = style->fontSize;
        float cursorX = bounds.left + paddingL + m_scrollOffset +
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

bool TextBox::OnEvent(const UIEvent& event) noexcept
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
        SetCursorPosition(pos);
        return true;
    }

    case UIEventType::Char:
        if (m_readOnly) return true;
        if (event.character >= 32 && m_text.length() < m_maxLength)
        {
            m_text.insert(m_cursorPos, 1, event.character);
            ++m_cursorPos;
            if (m_onTextChanged) m_onTextChanged(m_text);
            InvalidateVisual();
        }
        return true;

    case UIEventType::KeyDown:
    {
        switch (event.key)
        {
        case Input::KeyCode::Back:
            if (m_readOnly) return true;
            if (m_cursorPos > 0 && !m_text.empty())
            {
                m_text.erase(m_cursorPos - 1, 1);
                --m_cursorPos;
                if (m_onTextChanged) m_onTextChanged(m_text);
                InvalidateVisual();
            }
            return true;

        case Input::KeyCode::Delete:
            if (m_readOnly) return true;
            if (m_cursorPos < m_text.length())
            {
                m_text.erase(m_cursorPos, 1);
                if (m_onTextChanged) m_onTextChanged(m_text);
                InvalidateVisual();
            }
            return true;

        case Input::KeyCode::Left:
            if (m_cursorPos > 0) { --m_cursorPos; InvalidateVisual(); }
            return true;

        case Input::KeyCode::Right:
            if (m_cursorPos < m_text.length()) { ++m_cursorPos; InvalidateVisual(); }
            return true;

        case Input::KeyCode::Home:
            m_cursorPos = 0;
            InvalidateVisual();
            return true;

        case Input::KeyCode::End:
            m_cursorPos = m_text.length();
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
