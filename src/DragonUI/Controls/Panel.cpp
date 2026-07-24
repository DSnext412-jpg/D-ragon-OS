#include <DragonUI/Controls/Panel.hpp>
#include <DragonUI/Core/RenderContext.hpp>

namespace DragonOS::DragonUI {

void UIPanel::Render(RenderContext& ctx) noexcept
{
    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    if (m_background != Theme::SemanticColor::Transparent)
    {
        if (m_cornerRadius > 0.0f)
            ctx.FillRoundedRect(d2d, m_background, m_cornerRadius, m_cornerRadius);
        else
            ctx.FillRectangle(d2d, m_background);
    }

    if (m_borderThickness > 0.0f && m_borderColor != Theme::SemanticColor::Transparent)
    {
        if (m_cornerRadius > 0.0f)
            ctx.DrawRoundedRect(d2d, m_borderColor, m_cornerRadius, m_cornerRadius, m_borderThickness);
        else
            ctx.DrawRectangle(d2d, m_borderColor, m_borderThickness);
    }

    Container::Render(ctx);
}

} // namespace
