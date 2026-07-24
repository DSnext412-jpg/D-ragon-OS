#include <DragonUI/Controls/ProgressBar.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>
#include <cmath>

namespace DragonOS::DragonUI {

void UIProgressBar::SetValue(float value) noexcept
{
    m_value = std::clamp(value, m_min, m_max);
    InvalidateVisual();
}

void UIProgressBar::SetRange(float min, float max) noexcept
{
    m_min = min;
    m_max = max;
    m_value = std::clamp(m_value, m_min, m_max);
    InvalidateVisual();
}

float UIProgressBar::GetNormalizedValue() const noexcept
{
    if (m_max <= m_min) return 0.0f;
    return (m_value - m_min) / (m_max - m_min);
}

DesiredSize UIProgressBar::MeasureOverride(const LayoutSlot& available) noexcept
{
    return {std::min(200.0f, available.width), 6.0f};
}

void UIProgressBar::Render(RenderContext& ctx) noexcept
{
    Control::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);
    float radius = slot.height * 0.5f;

    ctx.FillRoundedRect(d2d, Theme::SemanticColor::Disabled, radius, radius);

    if (m_indeterminate)
    {
        float barW = slot.width * 0.3f;
        float phase = std::fmod(m_animTime, 2.0f) / 2.0f;
        float barX = slot.x + phase * (slot.width - barW);

        D2D1_RECT_F fill = {
            barX, d2d.top,
            barX + barW, d2d.bottom
        };
        ctx.FillRoundedRect(fill, Theme::SemanticColor::Accent, radius, radius);
    }
    else
    {
        float norm = GetNormalizedValue();
        if (norm > 0.0f)
        {
            float fillW = slot.width * norm;
            D2D1_RECT_F fill = {
                d2d.left, d2d.top,
                d2d.left + fillW, d2d.bottom
            };
            ctx.FillRoundedRect(fill, Theme::SemanticColor::Accent, radius, radius);
        }
    }
}

} // namespace
