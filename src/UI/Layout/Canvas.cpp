#include <UI/Layout/Canvas.hpp>
#include <limits>

namespace DragonOS::UI {

void Canvas::SetChildPosition(UIElement* child, float x, float y) noexcept
{
    if (!child) return;
    m_positions[child->GetId()] = {x, y};
    InvalidateLayout();
}

std::pair<float, float> Canvas::GetChildPosition(const UIElement* child) const noexcept
{
    if (!child) return {0, 0};
    auto it = m_positions.find(child->GetId());
    if (it != m_positions.end())
        return {it->second.x, it->second.y};
    return {0, 0};
}

D2D1_RECT_F Canvas::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
            continue;

        D2D1_RECT_F childAvail = {
            0, 0,
            (std::numeric_limits<float>::max)(),
            (std::numeric_limits<float>::max)()
        };
        child->Measure(childAvail);
    }

    return {0, 0, availW, availH};
}

void Canvas::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
            continue;

        const auto& ds = child->GetDesiredSize();
        float cw = ds.right - ds.left;
        float ch = ds.bottom - ds.top;

        auto it = m_positions.find(child->GetId());
        float px = it != m_positions.end() ? it->second.x : 0;
        float py = it != m_positions.end() ? it->second.y : 0;

        D2D1_RECT_F r = {left + px, top + py, left + px + cw, top + py + ch};
        child->Arrange(r);
    }
}

} // namespace
