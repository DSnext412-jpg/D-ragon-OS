#include <UI/Layout/WrapPanel.hpp>
#include <limits>
#include <algorithm>

namespace DragonOS::UI {

WrapPanel::WrapPanel(Orientation orientation) noexcept
    : m_orientation(orientation)
{
}

void WrapPanel::SetOrientation(Orientation orient) noexcept
{
    m_orientation = orient;
    InvalidateLayout();
}

D2D1_RECT_F WrapPanel::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;

    auto getChildW = [&](UIElement* c) -> float {
        if (m_itemWidth > 0) return m_itemWidth;
        const auto& ds = c->GetDesiredSize();
        return ds.right - ds.left;
    };

    auto getChildH = [&](UIElement* c) -> float {
        if (m_itemHeight > 0) return m_itemHeight;
        const auto& ds = c->GetDesiredSize();
        return ds.bottom - ds.top;
    };

    if (m_orientation == Orientation::Horizontal)
    {
        float totalW = 0, totalH = 0;
        float lineW = 0, lineH = 0;

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            float cw = m_itemWidth > 0 ? m_itemWidth : (std::numeric_limits<float>::max)();
            float ch = m_itemHeight > 0 ? m_itemHeight : (std::numeric_limits<float>::max)();
            D2D1_RECT_F childAvail = {0, 0, cw, ch};
            child->Measure(childAvail);

            cw = getChildW(child.get());
            ch = getChildH(child.get());

            if (lineW + cw > availW && lineW > 0)
            {
                totalW = (std::max)(totalW, lineW);
                totalH += lineH + m_spacing;
                lineW = 0;
                lineH = 0;
            }

            if (lineW > 0) lineW += m_spacing;
            lineW += cw;
            lineH = (std::max)(lineH, ch);
        }

        totalW = (std::max)(totalW, lineW);
        totalH += lineH;

        return {0, 0, totalW, totalH};
    }
    else
    {
        float totalW = 0, totalH = 0;
        float colW = 0, colH = 0;

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            float cw = m_itemWidth > 0 ? m_itemWidth : (std::numeric_limits<float>::max)();
            float ch = m_itemHeight > 0 ? m_itemHeight : (std::numeric_limits<float>::max)();
            D2D1_RECT_F childAvail = {0, 0, cw, ch};
            child->Measure(childAvail);

            cw = getChildW(child.get());
            ch = getChildH(child.get());

            if (colH + ch > availH && colH > 0)
            {
                totalH = (std::max)(totalH, colH);
                totalW += colW + m_spacing;
                colH = 0;
                colW = 0;
            }

            if (colH > 0) colH += m_spacing;
            colH += ch;
            colW = (std::max)(colW, cw);
        }

        totalH = (std::max)(totalH, colH);
        totalW += colW;

        return {0, 0, totalW, totalH};
    }
}

void WrapPanel::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;
    float contentW = (std::max)(0.0f, finalRect.right - finalRect.left);
    float contentH = (std::max)(0.0f, finalRect.bottom - finalRect.top);

    auto getChildW = [&](UIElement* c) -> float {
        if (m_itemWidth > 0) return m_itemWidth;
        const auto& ds = c->GetDesiredSize();
        return ds.right - ds.left;
    };

    auto getChildH = [&](UIElement* c) -> float {
        if (m_itemHeight > 0) return m_itemHeight;
        const auto& ds = c->GetDesiredSize();
        return ds.bottom - ds.top;
    };

    if (m_orientation == Orientation::Horizontal)
    {
        float lineTop = top;
        float lineH = 0;
        float offsetX = left;

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            float cw = getChildW(child.get());
            float ch = getChildH(child.get());

            if (offsetX + cw > left + contentW && offsetX > left)
            {
                offsetX = left;
                lineTop += lineH + m_spacing;
                lineH = 0;
            }

            D2D1_RECT_F r = {offsetX, lineTop, offsetX + cw, lineTop + ch};
            child->Arrange(r);
            offsetX += cw + m_spacing;
            lineH = (std::max)(lineH, ch);
        }
    }
    else
    {
        float colLeft = left;
        float colW = 0;
        float offsetY = top;

        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            float cw = getChildW(child.get());
            float ch = getChildH(child.get());

            if (offsetY + ch > top + contentH && offsetY > top)
            {
                offsetY = top;
                colLeft += colW + m_spacing;
                colW = 0;
            }

            D2D1_RECT_F r = {colLeft, offsetY, colLeft + cw, offsetY + ch};
            child->Arrange(r);
            offsetY += ch + m_spacing;
            colW = (std::max)(colW, cw);
        }
    }
}

} // namespace
