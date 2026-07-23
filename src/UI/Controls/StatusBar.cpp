#include <UI/Controls/StatusBar.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

StatusBar::StatusBar() noexcept
{
    SetStyle(UIStyle::DefaultStatusBar());
    SetAccessibilityRole(L"statusbar");
}

void StatusBar::SetText(std::wstring_view text) noexcept
{
    m_text = text;
    InvalidateVisual();
}

void StatusBar::SetRightText(std::wstring_view text) noexcept
{
    m_rightText = text;
    InvalidateVisual();
}

D2D1_RECT_F StatusBar::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availWidth = available.right - available.left;
    float availHeight = available.bottom - available.top;
    float height = (std::min)(MinHeight, availHeight);
    return {0, 0, availWidth, height};
}

void StatusBar::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors barColors;
    barColors.background = Theme::SemanticColor::WindowBackground;
    barColors.border = Theme::SemanticColor::WindowBorder;
    renderer.FillBackground(bounds, barColors, 0.0f, opacity);
    renderer.DrawBorder(bounds, barColors, 1.0f, 0.0f, opacity);

    const auto& textColors = style->ResolveState(ElementState::Normal);

    if (!m_text.empty())
    {
        D2D1_RECT_F textRect = {
            bounds.left + 8.0f, bounds.top + 3.0f,
            bounds.right * 0.6f, bounds.bottom - 3.0f
        };
        renderer.DrawText(m_text, textRect, textColors);
    }

    if (!m_rightText.empty())
    {
        float rightTextWidth = static_cast<float>(m_rightText.length()) * 7.0f;
        D2D1_RECT_F rightRect = {
            bounds.right - rightTextWidth - 8.0f, bounds.top + 3.0f,
            bounds.right - 8.0f, bounds.bottom - 3.0f
        };
        renderer.DrawText(m_rightText, rightRect, textColors);
    }
}

} // namespace
