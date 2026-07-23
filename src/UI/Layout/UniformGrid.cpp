#include <UI/Layout/UniformGrid.hpp>
#include <algorithm>
#include <cmath>

namespace DragonOS::UI {

D2D1_RECT_F UniformGrid::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;

    size_t visibleCount = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Collapsed)
            ++visibleCount;
    }

    if (visibleCount == 0)
        return {0, 0, 0, 0};

    size_t cols = (std::max)(m_columns, size_t{1});
    size_t rows = m_rows;
    if (rows == 0)
        rows = (visibleCount + cols - 1) / cols;

    float cellW = availW / static_cast<float>(cols);
    float cellH = availH / static_cast<float>(rows);

    float maxChildW = 0, maxChildH = 0;

    size_t idx = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
            continue;

        D2D1_RECT_F childAvail = {0, 0, cellW, cellH};
        child->Measure(childAvail);

        const auto& ds = child->GetDesiredSize();
        float cw = ds.right - ds.left;
        float ch = ds.bottom - ds.top;

        maxChildW = (std::max)(maxChildW, cw);
        maxChildH = (std::max)(maxChildH, ch);
        ++idx;
    }

    float totalW = cols * (std::max)(cellW, maxChildW);
    float totalH = rows * (std::max)(cellH, maxChildH);

    return {0, 0, totalW, totalH};
}

void UniformGrid::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;
    float contentW = (std::max)(0.0f, finalRect.right - finalRect.left);
    float contentH = (std::max)(0.0f, finalRect.bottom - finalRect.top);

    size_t visibleCount = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() != Visibility::Collapsed)
            ++visibleCount;
    }

    if (visibleCount == 0) return;

    size_t cols = (std::max)(m_columns, size_t{1});
    size_t rows = m_rows;
    if (rows == 0)
        rows = (visibleCount + cols - 1) / cols;

    float cellW = contentW / static_cast<float>(cols);
    float cellH = contentH / static_cast<float>(rows);

    size_t idx = 0;
    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed)
            continue;

        size_t visualIdx = idx + m_firstColumn;
        size_t col = visualIdx % cols;
        size_t row = visualIdx / cols;

        if (row >= rows)
            break;

        D2D1_RECT_F r = {
            left + col * cellW,
            top + row * cellH,
            left + (col + 1) * cellW,
            top + (row + 1) * cellH
        };
        child->Arrange(r);
        ++idx;
    }
}

} // namespace
