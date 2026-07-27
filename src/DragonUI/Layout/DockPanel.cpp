#include <DragonUI/Controls/DockPanel.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void UIDockPanel::SetChildDock(Element& child, Dock dock)
{
    m_dockMap[&child] = dock;
    InvalidateLayout();
}

Dock UIDockPanel::GetChildDock(const Element& child) const
{
    auto it = m_dockMap.find(&child);
    if (it != m_dockMap.end())
        return it->second;
    return Dock::Fill;
}

DesiredSize UIDockPanel::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);
    float remainingW = content.width;
    float remainingH = content.height;

    size_t childCount = m_children.size();
    size_t nonFillCount = childCount;
    if (m_lastChildFill && childCount > 0)
        nonFillCount = childCount - 1;

    for (size_t i = 0; i < nonFillCount; ++i)
    {
        auto& child = m_children[i];
        if (child->GetVisibility() == Visibility::Collapsed) continue;

        Dock dock = GetChildDock(*child);

        switch (dock)
        {
        case Dock::Left:
        case Dock::Right:
            child->Measure({content.x, content.y, remainingW, remainingH});
            remainingW -= (std::min)(child->GetDesiredSize().width, remainingW);
            break;

        case Dock::Top:
        case Dock::Bottom:
            child->Measure({content.x, content.y, remainingW, remainingH});
            remainingH -= (std::min)(child->GetDesiredSize().height, remainingH);
            break;

        default:
            break;
        }
    }

    if (m_lastChildFill && childCount > 0)
    {
        auto& last = m_children.back();
        if (last->GetVisibility() != Visibility::Collapsed)
            last->Measure({content.x, content.y, remainingW, remainingH});
    }

    return {content.width - remainingW, content.height - remainingH};
}

void UIDockPanel::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UIDockPanel::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    float left = finalSlot.x;
    float top = finalSlot.y;
    float right = finalSlot.x + finalSlot.width;
    float bottom = finalSlot.y + finalSlot.height;

    size_t childCount = m_children.size();
    size_t nonFillCount = childCount;
    if (m_lastChildFill && childCount > 0)
        nonFillCount = childCount - 1;

    for (size_t i = 0; i < nonFillCount; ++i)
    {
        auto& child = m_children[i];
        if (child->GetVisibility() == Visibility::Collapsed) continue;

        auto ds = child->GetDesiredSize();
        Dock dock = GetChildDock(*child);

        switch (dock)
        {
        case Dock::Left:
        {
            float childW = (std::min)(ds.width, right - left);
            child->Arrange({left, top, childW, bottom - top});
            left += childW;
            break;
        }
        case Dock::Top:
        {
            float childH = (std::min)(ds.height, bottom - top);
            child->Arrange({left, top, right - left, childH});
            top += childH;
            break;
        }
        case Dock::Right:
        {
            float childW = (std::min)(ds.width, right - left);
            child->Arrange({right - childW, top, childW, bottom - top});
            right -= childW;
            break;
        }
        case Dock::Bottom:
        {
            float childH = (std::min)(ds.height, bottom - top);
            child->Arrange({left, bottom - childH, right - left, childH});
            bottom -= childH;
            break;
        }
        default:
            child->Arrange({left, top, right - left, bottom - top});
            break;
        }
    }

    if (m_lastChildFill && childCount > 0)
    {
        auto& last = m_children.back();
        if (last->GetVisibility() != Visibility::Collapsed)
        {
            last->Arrange({left, top, (std::max)(0.0f, right - left), (std::max)(0.0f, bottom - top)});
        }
    }
}

void UIDockPanel::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

} // namespace
