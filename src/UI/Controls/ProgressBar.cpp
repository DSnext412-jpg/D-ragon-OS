#include <UI/Controls/ProgressBar.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>
#include <cmath>
#include <format>

namespace DragonOS::UI {

ProgressBar::ProgressBar() noexcept
{
    SetStyle(UIStyle::DefaultProgressBar());
    SetAccessibilityRole(L"progressbar");
}

D2D1_RECT_F ProgressBar::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    auto style = GetStyle();
    float paddingT = style ? style->paddingTop : 4.0f;
    float paddingB = style ? style->paddingBottom : 4.0f;

    float width = (std::max)(80.0f, available.right - available.left);
    float height = BarHeight + paddingT + paddingB;
    if (m_showLabel)
        height += 16.0f;
    height = (std::min)(height, available.bottom - available.top);

    return D2D1_RECT_F{0, 0, width, height};
}

void ProgressBar::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    ElementState state = GetState();
    if (!IsEnabled()) state = ElementState::Disabled;

    const auto& stateColors = style->ResolveState(state);
    float opacity = GetAnimatedOpacity() * GetOpacity();
    float cornerRadius = BarHeight * 0.5f;

    float paddingT = style->paddingTop;

    float barY = bounds.top + paddingT;
    float barHeight = BarHeight;
    if (m_showLabel)
    {
        barY += 16.0f;
    }

    float barWidth = bounds.right - bounds.left;
    D2D1_RECT_F trackRect = {
        bounds.left,
        barY,
        bounds.left + barWidth,
        barY + barHeight
    };

    StyleStateColors trackColors = stateColors;
    trackColors.background = Theme::SemanticColor::Disabled;
    renderer.FillBackground(trackRect, trackColors, cornerRadius, opacity);

    if (m_indeterminate)
    {
        static float animOffset = 0.0f;
        animOffset += 2.0f;
        if (animOffset > barWidth) animOffset = -30.0f;

        float barLeft = bounds.left + animOffset;
        float barRight = (std::min)(barLeft + 30.0f, bounds.right);
        D2D1_RECT_F barRect = {
            barLeft,
            barY,
            barRight,
            barY + barHeight
        };
        StyleStateColors barColors;
        barColors.background = Theme::SemanticColor::Accent;
        renderer.FillBackground(barRect, barColors, cornerRadius, opacity);
    }
    else
    {
        float fillWidth = m_value * barWidth;
        if (fillWidth > 0.0f)
        {
            D2D1_RECT_F fillRect = {
                bounds.left,
                barY,
                bounds.left + fillWidth,
                barY + barHeight
            };
            StyleStateColors fillColors;
            fillColors.background = Theme::SemanticColor::Accent;
            renderer.FillBackground(fillRect, fillColors, cornerRadius, opacity);
        }
    }

    if (m_showLabel)
    {
        StyleStateColors labelColors = stateColors;
        std::wstring label;
        if (m_indeterminate)
        {
            label = L"Working...";
        }
        else
        {
            label = std::format(L"{:.0f}%", m_value * 100.0f);
        }

        D2D1_RECT_F labelRect = {
            bounds.left,
            bounds.top,
            bounds.right,
            bounds.top + 16.0f
        };
        renderer.DrawText(label, labelRect, labelColors);
    }
}

} // namespace
