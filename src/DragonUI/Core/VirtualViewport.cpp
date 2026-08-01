#include <DragonUI/Core/VirtualViewport.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void VirtualViewport::SetViewportSize(float width, float height) noexcept
{
    m_viewportWidth = (std::max)(0.0f, width);
    m_viewportHeight = (std::max)(0.0f, height);
    ClampScroll();
}

void VirtualViewport::SetItemExtent(float width, float height) noexcept
{
    m_itemWidth = (std::max)(1.0f, width);
    m_itemHeight = (std::max)(1.0f, height);
}

void VirtualViewport::SetTotalRows(int64_t rows) noexcept
{
    m_totalRows = (std::max<int64_t>)(0, rows);
}

void VirtualViewport::SetTotalColumns(int64_t columns) noexcept
{
    m_columns = (std::max<int64_t>)(1, columns);
}

void VirtualViewport::SetScrollX(float x) noexcept
{
    m_scrollX = x;
    ClampScroll();
}

void VirtualViewport::SetScrollY(float y) noexcept
{
    m_scrollY = y;
    ClampScroll();
}

int64_t VirtualViewport::GetFirstVisibleRow() const noexcept
{
    if (m_itemHeight <= 0.0f)
        return 0;
    const int64_t row = static_cast<int64_t>(m_scrollY / m_itemHeight);
    return (std::max<int64_t>)(0, row);
}

int64_t VirtualViewport::GetLastVisibleRow() const noexcept
{
    if (m_totalRows == 0)
        return -1;

    if (m_itemHeight <= 0.0f)
        return m_totalRows - 1;

    const int64_t last = static_cast<int64_t>((m_scrollY + m_viewportHeight) / m_itemHeight);
    return (std::min)(m_totalRows - 1, last);
}

int64_t VirtualViewport::GetVisibleRowCount() const noexcept
{
    const int64_t first = GetFirstVisibleRow();
    const int64_t last = GetLastVisibleRow();
    return last >= first ? (last - first + 1) : 0;
}

float VirtualViewport::GetRowY(int64_t row) const noexcept
{
    return static_cast<float>(row) * m_itemHeight;
}

int64_t VirtualViewport::GetFirstVisibleColumn() const noexcept
{
    if (m_columns <= 1)
        return 0;
    if (m_itemWidth <= 0.0f)
        return 0;
    const int64_t col = static_cast<int64_t>(m_scrollX / m_itemWidth);
    return (std::max<int64_t>)(0, col);
}

int64_t VirtualViewport::GetLastVisibleColumn() const noexcept
{
    if (m_columns <= 1)
        return 0;
    if (m_itemWidth <= 0.0f)
        return m_columns - 1;
    const int64_t last = static_cast<int64_t>((m_scrollX + m_viewportWidth) / m_itemWidth);
    return (std::min)(m_columns - 1, last);
}

int64_t VirtualViewport::GetVisibleColumnCount() const noexcept
{
    const int64_t first = GetFirstVisibleColumn();
    const int64_t last = GetLastVisibleColumn();
    return last >= first ? (last - first + 1) : 0;
}

float VirtualViewport::GetColumnX(int64_t column) const noexcept
{
    return static_cast<float>(column) * m_itemWidth;
}

float VirtualViewport::GetTotalContentHeight() const noexcept
{
    return static_cast<float>(m_totalRows) * m_itemHeight;
}

float VirtualViewport::GetTotalContentWidth() const noexcept
{
    return static_cast<float>(m_columns) * m_itemWidth;
}

float VirtualViewport::GetMaxScrollY() const noexcept
{
    return (std::max)(0.0f, GetTotalContentHeight() - m_viewportHeight);
}

float VirtualViewport::GetMaxScrollX() const noexcept
{
    return (std::max)(0.0f, GetTotalContentWidth() - m_viewportWidth);
}

int64_t VirtualViewport::GetColumns() const noexcept
{
    return m_columns;
}

void VirtualViewport::SetColumns(int64_t columns) noexcept
{
    m_columns = (std::max<int64_t>)(1, columns);
}

void VirtualViewport::ClampScroll() noexcept
{
    m_scrollX = (std::clamp)(m_scrollX, 0.0f, GetMaxScrollX());
    m_scrollY = (std::clamp)(m_scrollY, 0.0f, GetMaxScrollY());
}

void VirtualViewport::Reset() noexcept
{
    m_scrollX = 0.0f;
    m_scrollY = 0.0f;
}

} // namespace DragonOS::DragonUI
