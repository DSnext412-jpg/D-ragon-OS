#include <UI/Controls/GridView.hpp>
#include <UI/Core/UIRenderer.hpp>
#include <UI/Core/UIStyle.hpp>
#include <algorithm>

namespace DragonOS::UI {

GridView::GridView() noexcept
{
    SetFocusable(true);
    SetStyle(UIStyle::DefaultListView());
    SetAccessibilityRole(L"grid");
}

void GridView::SetColumns(const std::vector<GridViewColumn>& columns) noexcept
{
    m_columns = columns;
    InvalidateLayout();
}

void GridView::SetRows(const std::vector<std::vector<std::wstring>>& rows) noexcept
{
    m_rows = rows;
    if (m_selectedRow >= static_cast<int>(m_rows.size()))
        m_selectedRow = m_rows.empty() ? -1 : 0;
    InvalidateLayout();
}

void GridView::SetSelectedRow(int index) noexcept
{
    if (index < -1 || index >= static_cast<int>(m_rows.size()))
        return;
    if (m_selectedRow != index)
    {
        m_selectedRow = index;
        if (m_onSelectionChanged)
            m_onSelectionChanged(m_selectedRow);
        InvalidateVisual();
    }
}

void GridView::SetScrollOffset(float offset) noexcept
{
    float contentHeight = static_cast<float>(m_rows.size()) * RowHeight;
    float viewHeight = GetHeight() - HeaderHeight;
    float maxOffset = (std::max)(0.0f, contentHeight - viewHeight);
    m_scrollOffset = (std::max)(0.0f, (std::min)(offset, maxOffset));
    InvalidateVisual();
}

D2D1_RECT_F GridView::MeasureOverride(const D2D1_RECT_F& available) noexcept
{
    float totalWidth = 0;
    for (const auto& col : m_columns)
        totalWidth += col.width;

    float totalHeight = HeaderHeight + static_cast<float>(m_rows.size()) * RowHeight;
    float width = (std::min)((std::max)(totalWidth, 100.0f), available.right - available.left);
    float height = (std::min)(totalHeight, available.bottom - available.top);

    return {0, 0, width, height};
}

void GridView::Render(UIRenderer& renderer) noexcept
{
    auto bounds = GetBounds();
    auto style = GetStyle();
    if (!style) return;

    float opacity = GetAnimatedOpacity() * GetOpacity();

    StyleStateColors bgColors;
    bgColors.background = Theme::SemanticColor::WindowBackground;
    renderer.FillBackground(bounds, bgColors, 0.0f, opacity);

    renderer.PushClip({bounds.left, bounds.top + HeaderHeight, bounds.right, bounds.bottom});

    float visibleStart = m_scrollOffset;
    float visibleEnd = m_scrollOffset + (bounds.bottom - bounds.top - HeaderHeight);
    int startIdx = static_cast<int>(visibleStart / RowHeight);
    int endIdx = static_cast<int>(visibleEnd / RowHeight) + 1;
    startIdx = (std::max)(0, startIdx);
    endIdx = (std::min)(endIdx, static_cast<int>(m_rows.size()));

    for (int i = startIdx; i < endIdx; ++i)
    {
        float y = bounds.top + HeaderHeight + i * RowHeight - m_scrollOffset;
        D2D1_RECT_F rowRect = {bounds.left, y, bounds.right, y + RowHeight};

        if (i % 2 == 1)
        {
            StyleStateColors altColors;
            altColors.background = Theme::SemanticColor::WindowBackground;
            renderer.FillBackground(rowRect, altColors, 0.0f, opacity * 0.5f);
        }

        if (i == m_selectedRow)
        {
            StyleStateColors selColors;
            selColors.background = Theme::SemanticColor::Selection;
            renderer.FillBackground(rowRect, selColors, 0.0f, opacity);
        }
        else if (i == m_hoveredRow)
        {
            StyleStateColors hoverColors;
            hoverColors.background = Theme::SemanticColor::Hover;
            renderer.FillBackground(rowRect, hoverColors, 0.0f, opacity);
        }

        float cellX = bounds.left;
        const auto& row = m_rows[i];
        const auto& textColors = style->ResolveState(i == m_selectedRow ? ElementState::Focused : ElementState::Normal);

        for (size_t c = 0; c < m_columns.size() && c < row.size(); ++c)
        {
            float cellW = m_columns[c].width;
            D2D1_RECT_F cellRect = {cellX, y, cellX + cellW, y + RowHeight};

            if (c > 0)
            {
                StyleStateColors lineColors;
                lineColors.background = Theme::SemanticColor::WindowBorder;
                renderer.FillBackground({cellX, y, cellX + 1.0f, y + RowHeight}, lineColors, 0.0f, opacity * 0.5f);
            }

            D2D1_RECT_F textRect = {cellRect.left + 4.0f, cellRect.top + 2.0f, cellRect.right - 4.0f, cellRect.bottom - 2.0f};
            renderer.DrawText(row[c], textRect, textColors);

            cellX += cellW;
        }
    }

    renderer.PopClip();

    float headerY = bounds.top;
    for (size_t c = 0; c < m_columns.size(); ++c)
    {
        float cellX = bounds.left;
        for (size_t i = 0; i < c; ++i)
            cellX += m_columns[i].width;

        D2D1_RECT_F headerRect = {cellX, headerY, cellX + m_columns[c].width, headerY + HeaderHeight};

        StyleStateColors headerColors;
        headerColors.background = Theme::SemanticColor::ExplorerToolbarBackground;
        headerColors.foreground = Theme::SemanticColor::TextPrimary;
        headerColors.border = Theme::SemanticColor::WindowBorder;
        renderer.FillBackground(headerRect, headerColors, 0.0f, opacity);
        renderer.DrawBorder(headerRect, headerColors, 1.0f, 0.0f, opacity);

        D2D1_RECT_F headerTextRect = {headerRect.left + 4.0f, headerRect.top + 3.0f, headerRect.right - 4.0f, headerRect.bottom - 3.0f};
        renderer.DrawText(m_columns[c].header, headerTextRect, headerColors);
    }

    if (GetState() == ElementState::Focused && m_selectedRow >= 0)
    {
        float selY = bounds.top + HeaderHeight + m_selectedRow * RowHeight - m_scrollOffset;
        D2D1_RECT_F focusRect = {bounds.left, selY, bounds.right, selY + RowHeight};
        renderer.DrawFocusIndicator(focusRect);
    }
}

bool GridView::OnEvent(const UIEvent& event) noexcept
{
    if (!IsEnabled()) return false;

    auto bounds = GetBounds();

    switch (event.type)
    {
    case UIEventType::MouseEnter:
        SetState(ElementState::Hover);
        return true;

    case UIEventType::MouseLeave:
        SetState(ElementState::Normal);
        m_hoveredRow = -1;
        InvalidateVisual();
        return true;

    case UIEventType::MouseMove:
    {
        if (event.y >= bounds.top + HeaderHeight && event.y <= bounds.bottom)
        {
            int idx = static_cast<int>((event.y - bounds.top - HeaderHeight + m_scrollOffset) / RowHeight);
            if (idx >= 0 && idx < static_cast<int>(m_rows.size()))
            {
                if (m_hoveredRow != idx)
                {
                    m_hoveredRow = idx;
                    InvalidateVisual();
                }
                return true;
            }
        }
        if (m_hoveredRow != -1)
        {
            m_hoveredRow = -1;
            InvalidateVisual();
        }
        return false;
    }

    case UIEventType::MouseDown:
    {
        if (event.y < bounds.top + HeaderHeight) return false;
        int idx = static_cast<int>((event.y - bounds.top - HeaderHeight + m_scrollOffset) / RowHeight);
        if (idx >= 0 && idx < static_cast<int>(m_rows.size()))
        {
            SetSelectedRow(idx);
            return true;
        }
        return false;
    }

    case UIEventType::Scroll:
    {
        SetScrollOffset(m_scrollOffset - event.wheelDelta * RowHeight * 0.5f);
        return true;
    }

    case UIEventType::KeyDown:
        switch (event.key)
        {
        case Input::KeyCode::Down:
            SetSelectedRow((std::min)(m_selectedRow + 1, static_cast<int>(m_rows.size()) - 1));
            return true;
        case Input::KeyCode::Up:
            SetSelectedRow((std::max)(m_selectedRow - 1, 0));
            return true;
        case Input::KeyCode::Home:
            SetSelectedRow(0);
            m_scrollOffset = 0;
            InvalidateVisual();
            return true;
        case Input::KeyCode::End:
            SetSelectedRow(static_cast<int>(m_rows.size()) - 1);
            InvalidateVisual();
            return true;
        default:
            return false;
        }

    case UIEventType::FocusGained:
        SetState(ElementState::Focused);
        return true;

    case UIEventType::FocusLost:
        SetState(ElementState::Normal);
        return true;

    default:
        return false;
    }
}

} // namespace
