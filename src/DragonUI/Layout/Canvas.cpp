#include <DragonUI/Controls/Canvas.hpp>

namespace DragonOS::DragonUI {

void UICanvas::SetChildPosition(Element& child, float left, float top)
{
    m_positions[&child] = {left, top};
    InvalidateLayout();
}

std::pair<float, float> UICanvas::GetChildPosition(const Element& child) const
{
    auto it = m_positions.find(&child);
    if (it != m_positions.end())
        return it->second;
    return {0, 0};
}

DesiredSize UICanvas::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);
    float maxX{};
    float maxY{};

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;

        child->Measure({content.x, content.y, FLT_MAX, FLT_MAX});
        auto ds = child->GetDesiredSize();

        auto it = m_positions.find(child.get());
        float childLeft = it != m_positions.end() ? it->second.first : 0;
        float childTop = it != m_positions.end() ? it->second.second : 0;

        float childRight = childLeft + ds.width;
        float childBottom = childTop + ds.height;

        if (childRight > maxX) maxX = childRight;
        if (childBottom > maxY) maxY = childBottom;
    }

    return {maxX, maxY};
}

void UICanvas::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UICanvas::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;

        auto ds = child->GetDesiredSize();
        auto it = m_positions.find(child.get());
        float childLeft = it != m_positions.end() ? it->second.first : 0;
        float childTop = it != m_positions.end() ? it->second.second : 0;

        child->Arrange({finalSlot.x + childLeft, finalSlot.y + childTop, ds.width, ds.height});
    }
}

void UICanvas::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

} // namespace
