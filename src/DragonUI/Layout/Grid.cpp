#include <DragonUI/Controls/Grid.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void UIGrid::AddRow(GridLength height, float minH, float maxH)
{
    m_rows.push_back({height, minH, maxH});
    InvalidateLayout();
}

void UIGrid::AddColumn(GridLength width, float minW, float maxW)
{
    m_cols.push_back({width, minW, maxW});
    InvalidateLayout();
}

void UIGrid::RemoveAllRows() noexcept
{
    m_rows.clear();
    InvalidateLayout();
}

void UIGrid::RemoveAllColumns() noexcept
{
    m_cols.clear();
    InvalidateLayout();
}

void UIGrid::SetChildPosition(Element& child, uint32_t row, uint32_t col)
{
    for (auto& info : m_childGridInfo)
    {
        if (info.element == &child)
        {
            info.row = row;
            info.col = col;
            InvalidateLayout();
            return;
        }
    }
    m_childGridInfo.push_back({&child, row, col});
    InvalidateLayout();
}

std::pair<uint32_t, uint32_t> UIGrid::GetChildPosition(const Element& child) const
{
    for (const auto& info : m_childGridInfo)
    {
        if (info.element == &child)
            return {info.row, info.col};
    }
    return {0, 0};
}

void UIGrid::SetRowSpacing(float spacing) noexcept
{
    if (m_rowSpacing != spacing)
    {
        m_rowSpacing = spacing;
        InvalidateLayout();
    }
}

void UIGrid::SetColumnSpacing(float spacing) noexcept
{
    if (m_colSpacing != spacing)
    {
        m_colSpacing = spacing;
        InvalidateLayout();
    }
}

void UIGrid::MeasureAutoRows(const LayoutSlot& slot, std::vector<float>& rowHeights)
{
    for (size_t r = 0; r < m_rows.size(); ++r)
    {
        if (m_rows[r].height.type != GridLength::Type::Auto)
            continue;

        float maxH{};
        for (const auto& info : m_childGridInfo)
        {
            if (info.row != r) continue;
            if (info.element->GetVisibility() == Visibility::Collapsed) continue;
            if (info.col >= m_cols.size()) continue;

            auto& col = m_cols[info.col];
            float childAvailW = col.actualWidth > 0 ? col.actualWidth : slot.width;
            info.element->Measure({slot.x, slot.y, childAvailW, FLT_MAX});
            auto ds = info.element->GetDesiredSize();
            if (ds.height > maxH) maxH = ds.height;
        }
        rowHeights[r] = std::clamp(maxH, m_rows[r].minHeight, m_rows[r].maxHeight);
    }
}

void UIGrid::MeasureAutoCols(const LayoutSlot& slot, std::vector<float>& colWidths)
{
    for (size_t c = 0; c < m_cols.size(); ++c)
    {
        if (m_cols[c].width.type != GridLength::Type::Auto)
            continue;

        float maxW{};
        for (const auto& info : m_childGridInfo)
        {
            if (info.col != c) continue;
            if (info.element->GetVisibility() == Visibility::Collapsed) continue;
            if (info.row >= m_rows.size()) continue;

            auto& row = m_rows[info.row];
            float childAvailH = row.actualHeight > 0 ? row.actualHeight : slot.height;
            info.element->Measure({slot.x, slot.y, FLT_MAX, childAvailH});
            auto ds = info.element->GetDesiredSize();
            if (ds.width > maxW) maxW = ds.width;
        }
        colWidths[c] = std::clamp(maxW, m_cols[c].minWidth, m_cols[c].maxWidth);
    }
}

void UIGrid::DistributeStarRows(float availableHeight, std::vector<float>& rowHeights)
{
    float totalStar{};
    float used{};
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        if (m_rows[i].height.type == GridLength::Type::Star)
            totalStar += m_rows[i].height.value;
        else
            used += rowHeights[i];
    }

    if (totalStar <= 0) return;

    float remaining = (std::max)(0.0f, availableHeight - used - m_rowSpacing * (m_rows.size() - 1));
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        if (m_rows[i].height.type != GridLength::Type::Star) continue;
        float h = remaining * (m_rows[i].height.value / totalStar);
        rowHeights[i] = std::clamp(h, m_rows[i].minHeight, m_rows[i].maxHeight);
    }
}

void UIGrid::DistributeStarCols(float availableWidth, std::vector<float>& colWidths)
{
    float totalStar{};
    float used{};
    for (size_t i = 0; i < m_cols.size(); ++i)
    {
        if (m_cols[i].width.type == GridLength::Type::Star)
            totalStar += m_cols[i].width.value;
        else
            used += colWidths[i];
    }

    if (totalStar <= 0) return;

    float remaining = (std::max)(0.0f, availableWidth - used - m_colSpacing * (m_cols.size() - 1));
    for (size_t i = 0; i < m_cols.size(); ++i)
    {
        if (m_cols[i].width.type != GridLength::Type::Star) continue;
        float w = remaining * (m_cols[i].width.value / totalStar);
        colWidths[i] = std::clamp(w, m_cols[i].minWidth, m_cols[i].maxWidth);
    }
}

DesiredSize UIGrid::MeasureOverride(const LayoutSlot& available) noexcept
{
    auto content = available.Inset(m_padding);

    size_t rowCount = m_rows.size();
    size_t colCount = m_cols.size();
    if (rowCount == 0) rowCount = 1;
    if (colCount == 0) colCount = 1;

    std::vector<float> rowHeights(rowCount, 0);
    std::vector<float> colWidths(colCount, 0);

    for (size_t i = 0; i < rowCount; ++i)
    {
        if (i < m_rows.size() && m_rows[i].height.type == GridLength::Type::Pixel)
            rowHeights[i] = m_rows[i].height.value;
    }
    for (size_t i = 0; i < colCount; ++i)
    {
        if (i < m_cols.size() && m_cols[i].width.type == GridLength::Type::Pixel)
            colWidths[i] = m_cols[i].width.value;
    }

    MeasureAutoRows(content, rowHeights);
    MeasureAutoCols(content, colWidths);
    DistributeStarRows(content.height, rowHeights);
    DistributeStarCols(content.width, colWidths);

    float totalW{};
    float totalH{};
    for (auto w : colWidths) totalW += w;
    for (auto h : rowHeights) totalH += h;
    if (colCount > 1) totalW += m_colSpacing * (colCount - 1);
    if (rowCount > 1) totalH += m_rowSpacing * (rowCount - 1);

    float offsetY{};
    for (size_t r = 0; r < rowCount; ++r)
    {
        if (r < m_rows.size())
        {
            m_rows[r].actualOffset = offsetY;
            m_rows[r].actualHeight = rowHeights[r];
        }
        offsetY += rowHeights[r] + m_rowSpacing;
    }
    float offsetX{};
    for (size_t c = 0; c < colCount; ++c)
    {
        if (c < m_cols.size())
        {
            m_cols[c].actualOffset = offsetX;
            m_cols[c].actualWidth = colWidths[c];
        }
        offsetX += colWidths[c] + m_colSpacing;
    }

    return {totalW, totalH};
}

void UIGrid::MeasureChildren(const LayoutSlot& available) noexcept
{
}

void UIGrid::ArrangeOverride(const LayoutSlot& finalSlot) noexcept
{
    size_t rowCount = m_rows.size();
    size_t colCount = m_cols.size();
    if (rowCount == 0) rowCount = 1;
    if (colCount == 0) colCount = 1;

    for (const auto& info : m_childGridInfo)
    {
        if (info.element->GetVisibility() == Visibility::Collapsed) continue;
        if (info.row >= m_rows.size() && m_rows.size() > 0) continue;
        if (info.col >= m_cols.size() && m_cols.size() > 0) continue;

        size_t r = info.row < rowCount ? info.row : 0;
        size_t c = info.col < colCount ? info.col : 0;

        float childX = finalSlot.x + m_cols[c].actualOffset;
        float childY = finalSlot.y + m_rows[r].actualOffset;
        float childW = m_cols[c].actualWidth;
        float childH = m_rows[r].actualHeight;

        info.element->Arrange({childX, childY, childW, childH});
    }
}

void UIGrid::ArrangeChildren(const LayoutSlot& finalSlot) noexcept
{
}

} // namespace
