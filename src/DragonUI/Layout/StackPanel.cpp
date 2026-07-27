#include <DragonUI/Controls/StackPanel.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

UIStackPanel::UIStackPanel(Orientation orient) noexcept
    : m_orientation(orient)
{
}

void UIStackPanel::SetOrientation(Orientation orient) noexcept
{
    if (m_orientation != orient)
    {
        m_orientation = orient;
        InvalidateLayout();
    }
}

void UIStackPanel::SetSpacing(float spacing) noexcept
{
    if (m_spacing != spacing)
    {
        m_spacing = spacing;
        InvalidateLayout();
    }
}

void UIStackPanel::SetHorizontalAlignment(Alignment align) noexcept
{
    if (m_hAlign != align)
    {
        m_hAlign = align;
        InvalidateLayout();
    }
}

void UIStackPanel::SetVerticalAlignment(Alignment align) noexcept
{
    if (m_vAlign != align)
    {
        m_vAlign = align;
        InvalidateLayout();
    }
}

DesiredSize UIStackPanel::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);
    float totalW{};
    float totalH{};
    float maxChildW{};

    if (m_orientation == Orientation::Vertical)
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;
            child->Measure({content.x, content.y, content.width, FLT_MAX});
            auto ds = child->GetDesiredSize();
            if (ds.width > maxChildW) maxChildW = ds.width;
            totalH += ds.height + m_spacing;
        }
        totalW = maxChildW;
        if (totalH > m_spacing) totalH -= m_spacing;
    }
    else
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;
            child->Measure({content.x, content.y, FLT_MAX, content.height});
            auto ds = child->GetDesiredSize();
            if (ds.height > totalH) totalH = ds.height;
            totalW += ds.width + m_spacing;
        }
        if (totalW > m_spacing) totalW -= m_spacing;
    }

    return {totalW, totalH};
}

void UIStackPanel::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UIStackPanel::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float x = finalSlot.x;
    float y = finalSlot.y;

    if (m_orientation == Orientation::Vertical)
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;
            auto ds = child->GetDesiredSize();
            float childW = ds.width;
            float childH = ds.height;

            if (m_hAlign == Alignment::Stretch)
                childW = finalSlot.width;
            else if (m_hAlign == Alignment::Center)
                x = finalSlot.x + (finalSlot.width - childW) * 0.5f;
            else if (m_hAlign == Alignment::End)
                x = finalSlot.x + finalSlot.width - childW;
            else
                x = finalSlot.x;

            child->Arrange({x, y, childW, childH});
            y += childH + m_spacing;
        }
    }
    else
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;
            auto ds = child->GetDesiredSize();
            float childW = ds.width;
            float childH = ds.height;

            if (m_vAlign == Alignment::Stretch)
                childH = finalSlot.height;
            else if (m_vAlign == Alignment::Center)
                y = finalSlot.y + (finalSlot.height - childH) * 0.5f;
            else if (m_vAlign == Alignment::End)
                y = finalSlot.y + finalSlot.height - childH;
            else
                y = finalSlot.y;

            child->Arrange({x, y, childW, childH});
            x += childW + m_spacing;
        }
    }
}

void UIStackPanel::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

} // namespace
