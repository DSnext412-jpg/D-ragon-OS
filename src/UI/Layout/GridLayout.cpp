#include <UI/Layout/GridLayout.hpp>
#include <limits>
#include <algorithm>

namespace DragonOS::UI {

void GridLayout::AddColumn(const GridDefinition& col) noexcept
{
    m_columns.push_back(col);
    InvalidateLayout();
}

void GridLayout::AddRow(const GridDefinition& row) noexcept
{
    m_rows.push_back(row);
    InvalidateLayout();
}

void GridLayout::SetColumns(const std::vector<GridDefinition>& cols) noexcept
{
    m_columns = cols;
    InvalidateLayout();
}

void GridLayout::SetRows(const std::vector<GridDefinition>& rows) noexcept
{
    m_rows = rows;
    InvalidateLayout();
}

void GridLayout::SetChildPosition(UIElement* child, size_t col, size_t row) noexcept
{
    if (!child) return;
    m_childCells[child->GetId()] = {col, row};
    InvalidateLayout();
}

void GridLayout::ClearDefinitions() noexcept
{
    m_columns.clear();
    m_rows.clear();
    InvalidateLayout();
}

D2D1_RECT_F GridLayout::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float availW = available.right - available.left;
    float availH = available.bottom - available.top;

    size_t colCount = (std::max)(m_columns.size(), size_t{1});
    size_t rowCount = (std::max)(m_rows.size(), size_t{1});

    if (m_columns.empty())
        m_columns.push_back({0, false, true});
    if (m_rows.empty())
        m_rows.push_back({0, false, true});

    m_columnWidths.assign(colCount, 0);
    m_rowHeights.assign(rowCount, 0);

    auto getCol = [&](UIElement* c) -> size_t {
        auto it = m_childCells.find(c->GetId());
        return it != m_childCells.end() ? it->second.col : 0;
    };
    auto getRow = [&](UIElement* c) -> size_t {
        auto it = m_childCells.find(c->GetId());
        return it != m_childCells.end() ? it->second.row : 0;
    };

    // Step 1: Fixed sizes from definitions
    for (size_t i = 0; i < colCount; ++i)
        if (!m_columns[i].isAuto && !m_columns[i].isStar)
            m_columnWidths[i] = m_columns[i].size;

    for (size_t i = 0; i < rowCount; ++i)
        if (!m_rows[i].isAuto && !m_rows[i].isStar)
            m_rowHeights[i] = m_rows[i].size;

    // Step 2: Measure auto columns
    for (size_t i = 0; i < colCount; ++i)
    {
        if (!m_columns[i].isAuto) continue;

        float maxW = 0;
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed) continue;
            if (getCol(child.get()) != i) continue;

            float cellW = m_columnWidths[i] > 0 ? m_columnWidths[i] : availW;
            float cellH = m_rowHeights[getRow(child.get())] > 0
                ? m_rowHeights[getRow(child.get())]
                : (std::numeric_limits<float>::max)();

            D2D1_RECT_F childAvail = {0, 0, cellW, cellH};
            child->Measure(childAvail);

            const auto& ds = child->GetDesiredSize();
            maxW = (std::max)(maxW, ds.right - ds.left);
        }
        m_columnWidths[i] = (std::max)(m_columnWidths[i], maxW);
    }

    // Step 3: Resolve star columns
    {
        float fixedW = 0;
        size_t starCount = 0;
        for (size_t i = 0; i < colCount; ++i)
        {
            if (m_columns[i].isStar)
                ++starCount;
            else
                fixedW += m_columnWidths[i];
        }
        float remaining = (std::max)(0.0f, availW - fixedW);
        if (starCount > 0 && remaining > 0)
        {
            float perStar = remaining / static_cast<float>(starCount);
            for (size_t i = 0; i < colCount; ++i)
                if (m_columns[i].isStar)
                    m_columnWidths[i] = perStar;
        }
    }

    // Step 4: Measure auto rows (columns are now final)
    for (size_t i = 0; i < rowCount; ++i)
    {
        if (!m_rows[i].isAuto) continue;

        float maxH = 0;
        for (const auto& child : m_children)
        {
            if (child->GetVisibility() == Visibility::Collapsed) continue;
            if (getRow(child.get()) != i) continue;

            float cellW = m_columnWidths[getCol(child.get())];
            float cellH = m_rowHeights[i] > 0 ? m_rowHeights[i] : (std::numeric_limits<float>::max)();

            D2D1_RECT_F childAvail = {0, 0, cellW, cellH};
            child->Measure(childAvail);

            const auto& ds = child->GetDesiredSize();
            maxH = (std::max)(maxH, ds.bottom - ds.top);
        }
        m_rowHeights[i] = (std::max)(m_rowHeights[i], maxH);
    }

    // Step 5: Resolve star rows
    {
        float fixedH = 0;
        size_t starCount = 0;
        for (size_t i = 0; i < rowCount; ++i)
        {
            if (m_rows[i].isStar)
                ++starCount;
            else
                fixedH += m_rowHeights[i];
        }
        float remaining = (std::max)(0.0f, availH - fixedH);
        if (starCount > 0 && remaining > 0)
        {
            float perStar = remaining / static_cast<float>(starCount);
            for (size_t i = 0; i < rowCount; ++i)
                if (m_rows[i].isStar)
                    m_rowHeights[i] = perStar;
        }
    }

    float totalW = 0, totalH = 0;
    for (auto w : m_columnWidths) totalW += w;
    for (auto h : m_rowHeights) totalH += h;

    return {0, 0, totalW, totalH};
}

void GridLayout::ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept
{
    float left = finalRect.left;
    float top = finalRect.top;

    size_t colCount = m_columnWidths.size();
    size_t rowCount = m_rowHeights.size();

    auto getCol = [&](UIElement* c) -> size_t {
        auto it = m_childCells.find(c->GetId());
        return it != m_childCells.end() ? it->second.col : 0;
    };
    auto getRow = [&](UIElement* c) -> size_t {
        auto it = m_childCells.find(c->GetId());
        return it != m_childCells.end() ? it->second.row : 0;
    };

    std::vector<float> colOffsets(colCount + 1, left);
    for (size_t i = 0; i < colCount; ++i)
        colOffsets[i + 1] = colOffsets[i] + m_columnWidths[i];

    std::vector<float> rowOffsets(rowCount + 1, top);
    for (size_t i = 0; i < rowCount; ++i)
        rowOffsets[i + 1] = rowOffsets[i] + m_rowHeights[i];

    for (const auto& child : m_children)
    {
        if (child->GetVisibility() == Visibility::Collapsed) continue;

        size_t c = getCol(child.get());
        size_t r = getRow(child.get());
        c = (std::min)(c, colCount - 1);
        r = (std::min)(r, rowCount - 1);

        const auto& ds = child->GetDesiredSize();
        float cellW = m_columnWidths[c];
        float cellH = m_rowHeights[r];

        D2D1_RECT_F cellRect = {
            colOffsets[c], rowOffsets[r],
            colOffsets[c] + cellW,
            rowOffsets[r] + cellH
        };

        child->Arrange(cellRect);
    }
}

} // namespace
