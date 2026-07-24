#include <DragonUI/Controls/Separator.hpp>
#include <DragonUI/Core/RenderContext.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

DesiredSize UISeparator::MeasureOverride(const LayoutSlot& available) noexcept
{
    if (m_orientation == Orientation::Horizontal)
        return {std::min(available.width, 200.0f), m_thickness};
    else
        return {m_thickness, std::min(available.height, 200.0f)};
}

void UISeparator::Render(RenderContext& ctx) noexcept
{
    Element::Render(ctx);

    auto slot = GetBounds();
    auto d2d = static_cast<D2D1_RECT_F>(slot);

    ctx.FillRectangle(d2d, m_color);
}

} // namespace
