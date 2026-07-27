#include <DragonUI/Controls/WrapPanel.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UIWrapPanel::UIWrapPanel(Orientation orient) noexcept
    : m_orientation(orient)
{
}

void UIWrapPanel::SetOrientation(Orientation orient) noexcept
{
    if (m_orientation != orient)
    {
        m_orientation = orient;
        InvalidateLayout();
    }
}

void UIWrapPanel::SetItemWidth(float w) noexcept
{
    if (m_itemWidth != w)
    {
        m_itemWidth = w;
        InvalidateLayout();
    }
}

void UIWrapPanel::SetItemHeight(float h) noexcept
{
    if (m_itemHeight != h)
    {
        m_itemHeight = h;
        InvalidateLayout();
    }
}

void UIWrapPanel::SetHorizontalSpacing(float s) noexcept
{
    if (m_hSpacing != s)
    {
        m_hSpacing = s;
        InvalidateLayout();
    }
}

void UIWrapPanel::SetVerticalSpacing(float s) noexcept
{
    if (m_vSpacing != s)
    {
        m_vSpacing = s;
        InvalidateLayout();
    }
}

DesiredSize UIWrapPanel::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;
        float childW = m_itemWidth > 0 ? m_itemWidth : content.width;
        float childH = m_itemHeight > 0 ? m_itemHeight : content.height;
        child->Measure({content.x, content.y, childW, childH});
    }
    return {0, 0};
}

void UIWrapPanel::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UIWrapPanel::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    if (m_orientation == Orientation::Horizontal)
    {
        float x = finalSlot.x;
        float y = finalSlot.y;
        float rowHeight{};

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed) continue;

            float childW = m_itemWidth > 0 ? m_itemWidth : child->GetDesiredSize().width;
            float childH = m_itemHeight > 0 ? m_itemHeight : child->GetDesiredSize().height;

            if (x + childW > finalSlot.x + finalSlot.width && x > finalSlot.x)
            {
                x = finalSlot.x;
                y += rowHeight + m_vSpacing;
                rowHeight = 0;
            }

            child->Arrange({x, y, childW, childH});
            x += childW + m_hSpacing;
            if (childH > rowHeight) rowHeight = childH;
        }
    }
    else
    {
        float x = finalSlot.x;
        float y = finalSlot.y;
        float colWidth{};

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed) continue;

            float childW = m_itemWidth > 0 ? m_itemWidth : child->GetDesiredSize().width;
            float childH = m_itemHeight > 0 ? m_itemHeight : child->GetDesiredSize().height;

            if (y + childH > finalSlot.y + finalSlot.height && y > finalSlot.y)
            {
                y = finalSlot.y;
                x += colWidth + m_hSpacing;
                colWidth = 0;
            }

            child->Arrange({x, y, childW, childH});
            y += childH + m_vSpacing;
            if (childW > colWidth) colWidth = childW;
        }
    }
}

void UIWrapPanel::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

} // namespace
