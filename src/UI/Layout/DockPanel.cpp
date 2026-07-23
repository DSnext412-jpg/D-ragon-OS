#include <UI/Layout/DockPanel.hpp>
#include <limits>
#include <algorithm>

namespace DragonOS::UI {

void DockPanel::SetChildDock(UIElement* child, DockPosition dock) noexcept
{
    if (!child) return;
    m_childDock[child->GetId()] = dock;
    InvalidateLayout();
}

D2D1_RECT_F DockPanel::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;

    float usedW = 0, usedH = 0;
    float maxW = 0, maxH = 0;

    size_t childCount = m_children.size();
    size_t idx = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
        {
            ++idx;
            continue;
        }

        bool isLast = (idx == childCount - 1);
        DockPosition dock = DockPosition::Fill;
        auto it = m_childDock.find(child->GetId());
        if (it != m_childDock.end())
            dock = it->second;

        if (isLast && m_lastChildFill)
            dock = DockPosition::Fill;

        float remainingW = (std::max)(0.0f, availW - usedW);
        float remainingH = (std::max)(0.0f, availH - usedH);

        D2D1_RECT_F childAvail{};
        switch (dock)
        {
        case DockPosition::Left:
        case DockPosition::Right:
            childAvail = {0, 0, (std::numeric_limits<float>::max)(), remainingH};
            break;
        case DockPosition::Top:
        case DockPosition::Bottom:
            childAvail = {0, 0, remainingW, (std::numeric_limits<float>::max)()};
            break;
        case DockPosition::Fill:
            childAvail = {0, 0, remainingW, remainingH};
            break;
        }

        child->Measure(childAvail);
        const auto& ds = child->GetDesiredSize();
        float cw = ds.right - ds.left;
        float ch = ds.bottom - ds.top;

        switch (dock)
        {
        case DockPosition::Left:
        case DockPosition::Right:
            usedW += cw;
            maxH = (std::max)(maxH, ch);
            break;
        case DockPosition::Top:
        case DockPosition::Bottom:
            usedH += ch;
            maxW = (std::max)(maxW, cw);
            break;
        case DockPosition::Fill:
            maxW = (std::max)(maxW, cw);
            maxH = (std::max)(maxH, ch);
            break;
        }

        ++idx;
    }

    float totalW = (std::max)(usedW, maxW);
    float totalH = (std::max)(usedH, maxH);

    return {0, 0, totalW, totalH};
}

void DockPanel::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;
    float right = finalRect.right;
    float bottom = finalRect.bottom;

    size_t childCount = m_children.size();
    size_t idx = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
        {
            ++idx;
            continue;
        }

        bool isLast = (idx == childCount - 1);
        DockPosition dock = DockPosition::Fill;
        auto it = m_childDock.find(child->GetId());
        if (it != m_childDock.end())
            dock = it->second;

        if (isLast && m_lastChildFill)
            dock = DockPosition::Fill;

        const auto& ds = child->GetDesiredSize();
        float cw = ds.right - ds.left;
        float ch = ds.bottom - ds.top;

        D2D1_RECT_F childRect{};
        switch (dock)
        {
        case DockPosition::Left:
            childRect = {left, top, left + cw, bottom};
            left += cw;
            break;
        case DockPosition::Top:
            childRect = {left, top, right, top + ch};
            top += ch;
            break;
        case DockPosition::Right:
            childRect = {right - cw, top, right, bottom};
            right -= cw;
            break;
        case DockPosition::Bottom:
            childRect = {left, bottom - ch, right, bottom};
            bottom -= ch;
            break;
        case DockPosition::Fill:
            childRect = {left, top, right, bottom};
            break;
        }

        child->Arrange(childRect);
        ++idx;
    }
}

} // namespace
