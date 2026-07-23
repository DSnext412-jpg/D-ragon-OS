#include <UI/Layout/StackPanel.hpp>
#include <limits>

namespace DragonOS::UI {

StackPanel::StackPanel(Orientation orientation) noexcept
    : m_orientation(orientation)
{
}

void StackPanel::SetOrientation(Orientation orient) noexcept
{
    m_orientation = orient;
    InvalidateLayout();
}

D2D1_RECT_F StackPanel::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;
    float totalW = 0, totalH = 0;
    bool first = true;

    if (m_orientation == Orientation::Vertical)
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            D2D1_RECT_F childAvail = {0, 0, availW, (std::numeric_limits<float>::max)()};
            child->Measure(childAvail);

            const auto& ds = child->GetDesiredSize();
            float cw = ds.right - ds.left;
            float ch = ds.bottom - ds.top;

            totalW = (std::max)(totalW, cw);
            if (!first) totalH += m_spacing;
            totalH += ch;
            first = false;
        }
    }
    else
    {
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            D2D1_RECT_F childAvail = {0, 0, (std::numeric_limits<float>::max)(), availH};
            child->Measure(childAvail);

            const auto& ds = child->GetDesiredSize();
            float cw = ds.right - ds.left;
            float ch = ds.bottom - ds.top;

            if (!first) totalW += m_spacing;
            totalW += cw;
            totalH = (std::max)(totalH, ch);
            first = false;
        }
    }

    return {0, 0, totalW, totalH};
}

void StackPanel::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;
    float contentW = (std::max)(0.0f, finalRect.right - finalRect.left);
    float contentH = (std::max)(0.0f, finalRect.bottom - finalRect.top);

    if (m_orientation == Orientation::Vertical)
    {
        float offsetY = top;
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            const auto& ds = child->GetDesiredSize();
            float ch = ds.bottom - ds.top;
            D2D1_RECT_F r = {left, offsetY, left + contentW, offsetY + ch};
            child->Arrange(r);
            offsetY += ch + m_spacing;
        }
    }
    else
    {
        float offsetX = left;
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed)
                continue;

            const auto& ds = child->GetDesiredSize();
            float cw = ds.right - ds.left;
            D2D1_RECT_F r = {offsetX, top, offsetX + cw, top + contentH};
            child->Arrange(r);
            offsetX += cw + m_spacing;
        }
    }
}

} // namespace
