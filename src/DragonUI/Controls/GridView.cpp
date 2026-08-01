#include <DragonUI/Controls/GridView.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <string>

namespace DragonOS::DragonUI {

namespace {
std::wstring GridToLower(std::wstring_view text)
{
    std::wstring result(text);
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return result;
}

int GridCompareText(const std::wstring& a, const std::wstring& b)
{
    const std::wstring la = GridToLower(a);
    const std::wstring lb = GridToLower(b);
    const int cmp = la.compare(lb);
    return cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
}
} // namespace

UIGridView::UIGridView() noexcept
{
    m_focusable = true;
    SetMinSize(200.0f, 140.0f);
    m_selection.SetMode(SelectionMode::Multi);
    m_selection.SetOnChanged([this](const SelectionManager& sm) { OnSelectionChanged(sm); });
}

// ── Data ─────────────────────────────────────────────────────────────

void UIGridView::SetRowSource(std::shared_ptr<VirtualItemSource> source) noexcept
{
    m_source = std::move(source);
    m_selection.Clear();
    RebuildRows();
    InvalidateLayout();
}

void UIGridView::AddColumn(Column column) noexcept
{
    m_columns.push_back(std::move(column));
    InvalidateVisual();
}

void UIGridView::ClearColumns() noexcept
{
    m_columns.clear();
    InvalidateVisual();
}

void UIGridView::SetRowHeight(float height) noexcept
{
    m_rowHeight = (std::max)(16.0f, height);
    InvalidateLayout();
}

void UIGridView::SetHeaderVisible(bool visible) noexcept
{
    m_headerVisible = visible;
    InvalidateLayout();
}

void UIGridView::SetAlternatingRowColors(bool enabled) noexcept
{
    m_alternatingRowColors = enabled;
    InvalidateVisual();
}

// ── Sorting ──────────────────────────────────────────────────────────

void UIGridView::SetSortColumn(size_t column, SortDirection direction) noexcept
{
    m_sortedColumn = column;
    m_sortDirection = direction;
    SortRows();
    InvalidateVisual();
}

void UIGridView::SortRows() noexcept
{
    if (m_rows.size() < 2 || m_sortedColumn == static_cast<size_t>(-1) ||
        m_sortedColumn >= m_columns.size())
        return;

    const Comparer cmp = m_columns[m_sortedColumn].comparer
        ? m_columns[m_sortedColumn].comparer
        : [this](const std::any& a, const std::any& b) {
              return GridCompareText(GetCellText(a, m_sortedColumn), GetCellText(b, m_sortedColumn));
          };

    std::stable_sort(m_rows.begin(), m_rows.end(), [this, &cmp](int64_t a, int64_t b) {
        const std::any va = m_source ? m_source->GetItem(a) : std::any{};
        const std::any vb = m_source ? m_source->GetItem(b) : std::any{};
        int result = cmp(va, vb);
        if (m_sortDirection == SortDirection::Descending)
            result = -result;
        return result < 0;
    });
}

// ── Selection / activation ───────────────────────────────────────────

std::any UIGridView::GetRowItem(int64_t displayRow) const noexcept
{
    return FetchRow(displayRow);
}

void UIGridView::Refresh() noexcept
{
    RebuildRows();
    InvalidateVisual();
}

// ── Scrolling ────────────────────────────────────────────────────────

void UIGridView::SetScrollOffset(float x, float y) noexcept
{
    m_scrollX = x;
    m_scrollY = y;
    ClampScroll();
    InvalidateVisual();
}

void UIGridView::ScrollToRow(int64_t displayRow) noexcept
{
    EnsureVisible(displayRow);
}

// ── Hit testing ──────────────────────────────────────────────────────

int64_t UIGridView::HitTestRowAt(float x, float y) const noexcept
{
    const float localX = x - GetX();
    const float localY = y - GetY();

    const float contentWidth = GetWidth() - ScrollBarSize;
    const float contentHeight = GetHeight() - ScrollBarSize;
    if (localX < 0.0f || localX >= contentWidth || localY < 0.0f || localY >= contentHeight)
        return -1;

    const float headerOffset = m_headerVisible ? m_headerHeight : 0.0f;
    if (localY < headerOffset)
        return -1;

    const int64_t row = static_cast<int64_t>((localY - headerOffset + m_scrollY) / m_rowHeight);
    return (row >= 0 && row < GetRowCount()) ? row : -1;
}

size_t UIGridView::HitTestColumnAt(float x) const noexcept
{
    const float localX = x - GetX() + m_scrollX;
    if (localX < 0.0f)
        return static_cast<size_t>(-1);

    float acc = 0.0f;
    for (size_t c = 0; c < m_columns.size(); ++c)
    {
        acc += m_columns[c].width;
        if (localX < acc)
            return c;
    }
    return static_cast<size_t>(-1);
}

// ── Layout ───────────────────────────────────────────────────────────

DesiredSize UIGridView::MeasureOverride(const LayoutSlot& /*available*/) noexcept
{
    return { m_minW > 0.0f ? m_minW : 320.0f, m_minH > 0.0f ? m_minH : 200.0f };
}

// ── Rendering ────────────────────────────────────────────────────────

void UIGridView::Render(RenderContext& ctx) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;
    ClampScroll();

    const auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::WindowBackground);

    ctx.PushClip(d2d);

    const float contentX = GetX();
    const float contentY = GetY();
    const float contentWidth = GetWidth() - ScrollBarSize;
    const float contentHeight = GetHeight() - ScrollBarSize;

    if (m_headerVisible)
        RenderHeader(ctx, contentX, contentY);

    const float headerOffset = m_headerVisible ? m_headerHeight : 0.0f;
    const int64_t totalRows = GetRowCount();

    if (totalRows > 0)
    {
        const int64_t first = static_cast<int64_t>(m_scrollY / m_rowHeight);
        const int64_t last = static_cast<int64_t>((m_scrollY + (contentHeight - headerOffset)) / m_rowHeight);

        for (int64_t row = first; row <= (std::min)(last, totalRows - 1); ++row)
        {
            const float rowTop = contentY + headerOffset + row * m_rowHeight - m_scrollY;
            RenderRow(ctx, contentX, contentY, row, rowTop);
        }
    }
    else
    {
        ctx.DrawText(L"No rows", { contentX + 12.0f, contentY + headerOffset + 8.0f,
                                   contentX + 200.0f, contentY + headerOffset + 28.0f },
            Theme::SemanticColor::PlaceholderText);
    }

    // Grid lines between columns.
    if (!m_columns.empty())
    {
        float acc = 0.0f;
        for (size_t c = 0; c + 1 < m_columns.size(); ++c)
        {
            acc += m_columns[c].width;
            const float gx = contentX + acc - m_scrollX;
            if (gx < contentX || gx > contentX + contentWidth)
                continue;
            ctx.FillRectangle({ gx, contentY + headerOffset, gx + 1.0f, contentY + contentHeight },
                Theme::SemanticColor::MenuSeparator, 0.35f);
        }
    }

    ctx.PopClip();
    RenderScrollBars(ctx);
}

void UIGridView::RenderHeader(RenderContext& ctx, float contentX, float contentY) noexcept
{
    const D2D1_RECT_F headerRect = {
        contentX, contentY,
        contentX + GetWidth() - ScrollBarSize, contentY + m_headerHeight
    };
    ctx.FillRectangle(headerRect, Theme::SemanticColor::WindowTitleBar);

    for (size_t c = 0; c < m_columns.size(); ++c)
    {
        const float cx = contentX + GetColumnX(c) - m_scrollX;
        if (cx + m_columns[c].width < contentX || cx > contentX + GetWidth() - ScrollBarSize)
            continue;

        const D2D1_RECT_F cell = { cx + 8.0f, contentY + 4.0f,
                                   cx + m_columns[c].width - 6.0f, contentY + m_headerHeight - 4.0f };

        Theme::SemanticColor textColor = Theme::SemanticColor::TextSecondary;
        if (c == m_sortedColumn && m_sortDirection != SortDirection::None)
        {
            textColor = Theme::SemanticColor::Accent;
            std::wstring_view arrow = m_sortDirection == SortDirection::Ascending ? L"\x25B2" : L"\x25BC";
            const D2D1_RECT_F arrowRect = { cx + m_columns[c].width - 18.0f, contentY + 4.0f,
                                            cx + m_columns[c].width - 4.0f, contentY + m_headerHeight - 4.0f };
            ctx.DrawText(arrow, arrowRect, Theme::SemanticColor::Accent);
        }

        ctx.DrawText(m_columns[c].title, cell, textColor);

        if (c + 1 < m_columns.size())
        {
            ctx.FillRectangle({ cx + m_columns[c].width - 1.0f, contentY + 2.0f,
                                cx + m_columns[c].width + 1.0f, contentY + m_headerHeight - 2.0f },
                Theme::SemanticColor::MenuSeparator, 0.6f);
        }
    }
}

void UIGridView::RenderRow(RenderContext& ctx, float contentX, float contentY, int64_t displayRow, float rowTop) noexcept
{
    const float contentWidth = GetWidth() - ScrollBarSize;
    const float contentHeight = GetHeight() - ScrollBarSize;
    if (rowTop + m_rowHeight < contentY || rowTop > contentY + contentHeight)
        return;

    const LayoutSlot slot{ contentX, rowTop, contentWidth, m_rowHeight };
    const bool selected = m_selection.IsSelected(displayRow);
    const bool hovered = (displayRow == m_hoveredRow);
    const bool zebra = m_alternatingRowColors && (displayRow % 2 == 1);

    if (selected)
        ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Selection);
    else if (hovered)
        ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover);
    else if (zebra)
        ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover, 0.35f);

    const ItemVisualState state{ selected, hovered, GetControlState() == ControlState::Focused, m_enabled };

    for (size_t c = 0; c < m_columns.size(); ++c)
    {
        const float cx = contentX + GetColumnX(c) - m_scrollX;
        if (cx + m_columns[c].width < contentX || cx > contentX + contentWidth)
            continue;

        const std::wstring text = GetCellText(displayRow, c);
        const D2D1_RECT_F cell = { cx + 8.0f, slot.y + 3.0f,
                                   cx + m_columns[c].width - 6.0f, slot.y + slot.height - 3.0f };
        Theme::SemanticColor textColor = Theme::SemanticColor::TextPrimary;
        if (m_columns[c].getColor)
        {
            const std::any value = FetchRow(displayRow);
            textColor = m_columns[c].getColor(value);
        }
        ctx.DrawText(text, cell, textColor);
    }
}

void UIGridView::RenderScrollBars(RenderContext& ctx) noexcept
{
    // Vertical scrollbar.
    {
        const float totalH = GetRowCount() * m_rowHeight + (m_headerVisible ? m_headerHeight : 0.0f);
        const float viewportH = GetHeight() - ScrollBarSize;
        const float maxScroll = (std::max)(0.0f, totalH - viewportH);
        if (maxScroll > 0.0f)
        {
            const float contentH = viewportH + maxScroll;
            const float thumbSize = (std::max)(20.0f, (viewportH / contentH) * viewportH);
            const float thumbPos = (m_scrollY / maxScroll) * (viewportH - thumbSize);

            ctx.FillRectangle({ GetX() + GetWidth() - ScrollBarSize, GetY(),
                                GetX() + GetWidth(), GetY() + viewportH },
                Theme::SemanticColor::ControlFill, 0.5f);
            ctx.FillRoundedRect({ GetX() + GetWidth() - ScrollBarSize + 1.0f, GetY() + thumbPos,
                                  GetX() + GetWidth() - 1.0f, GetY() + thumbPos + thumbSize },
                Theme::SemanticColor::Accent, 3.0f, 3.0f, 0.85f);
        }
    }

    // Horizontal scrollbar.
    {
        const float totalW = GetTotalColumnWidth();
        const float viewportW = GetWidth() - ScrollBarSize;
        const float maxScroll = (std::max)(0.0f, totalW - viewportW);
        if (maxScroll > 0.0f)
        {
            const float contentW = viewportW + maxScroll;
            const float thumbSize = (std::max)(20.0f, (viewportW / contentW) * viewportW);
            const float thumbPos = (m_scrollX / maxScroll) * (viewportW - thumbSize);

            ctx.FillRectangle({ GetX(), GetY() + GetHeight() - ScrollBarSize,
                                GetX() + viewportW, GetY() + GetHeight() },
                Theme::SemanticColor::ControlFill, 0.5f);
            ctx.FillRoundedRect({ GetX() + thumbPos, GetY() + GetHeight() - ScrollBarSize + 1.0f,
                                  GetX() + thumbPos + thumbSize, GetY() + GetHeight() - 1.0f },
                Theme::SemanticColor::Accent, 3.0f, 3.0f, 0.85f);
        }
    }
}

// ── Mouse ─────────────────────────────────────────────────────────────

bool UIGridView::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::MouseMove && std::abs(args.wheelDelta) > 0.1f)
    {
        const float delta = args.wheelDelta > 0.0f ? -1.0f : 1.0f;
        m_scrollY += delta * m_rowHeight * 3.0f;
        ClampScroll();
        InvalidateVisual();
        return true;
    }

    const float localX = args.x - GetX();
    const float localY = args.y - GetY();

    switch (type)
    {
    case EventType::MouseMove:
        m_hoveredRow = HitTestRowAt(args.x, args.y);
        m_hoveredColumn = HitTestColumnAt(args.x);
        if (m_columnResizing && m_dragColumn >= 0)
        {
            const size_t col = static_cast<size_t>(m_dragColumn);
            if (col < m_columns.size())
            {
                m_columns[col].width = (std::max)(MinColumnWidth, localX - GetColumnX(col) + m_scrollX);
                InvalidateVisual();
            }
            return true;
        }
        InvalidateVisual();
        return true;

    case EventType::MouseDown:
        if (args.button == Input::MouseButton::Left)
        {
            if (m_headerVisible && localY < m_headerHeight)
            {
                for (size_t c = 0; c + 1 < m_columns.size(); ++c)
                {
                    const float dividerX = GetColumnX(c) + m_columns[c].width - m_scrollX;
                    if (std::abs(localX - dividerX) <= 4.0f)
                    {
                        m_columnResizing = true;
                        m_dragColumn = static_cast<int64_t>(c);
                        return true;
                    }
                }

                const size_t col = HitTestColumnAt(args.x);
                if (col < m_columns.size() && m_columns[col].sortable)
                {
                    const SortDirection dir =
                        m_sortedColumn == col && m_sortDirection == SortDirection::Ascending
                            ? SortDirection::Descending
                            : SortDirection::Ascending;
                    SetSortColumn(col, dir);
                    return true;
                }
                return true;
            }

            const int64_t row = HitTestRowAt(args.x, args.y);
            if (row >= 0)
            {
                m_selection.OnPointerClick(row, args.ctrl, args.shift);
                InvalidateVisual();
                return true;
            }

            if (m_selection.GetSelectedCount() > 0)
            {
                m_selection.Clear();
                InvalidateVisual();
            }
            return true;
        }
        return false;

    case EventType::MouseUp:
        if (m_columnResizing)
        {
            m_columnResizing = false;
            m_dragColumn = -1;
            return true;
        }
        return false;

    case EventType::DoubleClick:
    {
        const int64_t row = HitTestRowAt(args.x, args.y);
        const size_t col = HitTestColumnAt(args.x);
        if (row >= 0 && m_onCellActivated)
        {
            m_onCellActivated(*this, row, col);
            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

bool UIGridView::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type != EventType::KeyDown)
        return false;

    const int64_t total = GetRowCount();
    if (total == 0)
        return false;

    auto current = m_selection.GetCurrent();

    auto move = [&](int64_t target) {
        target = (std::clamp)(target, int64_t{ 0 }, total - 1);
        m_selection.OnKeyMove(target, args.ctrl, args.shift);
        EnsureVisible(target);
        InvalidateVisual();
    };

    switch (args.key)
    {
    case Input::KeyCode::Down:
        move(current < 0 ? 0 : current + 1);
        return true;

    case Input::KeyCode::Up:
        move(current < 0 ? 0 : current - 1);
        return true;

    case Input::KeyCode::PageDown:
    {
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_rowHeight) - 1);
        move(current < 0 ? 0 : current + page);
        return true;
    }

    case Input::KeyCode::PageUp:
    {
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_rowHeight) - 1);
        move(current < 0 ? 0 : current - page);
        return true;
    }

    case Input::KeyCode::Home:
        move(0);
        return true;

    case Input::KeyCode::End:
        move(total - 1);
        return true;

    case Input::KeyCode::A:
        if (args.ctrl)
        {
            m_selection.SelectAll();
            InvalidateVisual();
            return true;
        }
        return false;

    case Input::KeyCode::Return:
        if (current >= 0 && m_onCellActivated)
        {
            m_onCellActivated(*this, current, m_sortedColumn);
            return true;
        }
        return false;

    default:
        return false;
    }
}

bool UIGridView::OnFocusEvent(const FocusEventArgs&) noexcept
{
    InvalidateVisual();
    return false;
}

// ── Private helpers ───────────────────────────────────────────────────

void UIGridView::RebuildRows() noexcept
{
    m_rows.clear();
    const int64_t count = m_source ? m_source->GetCount() : 0;
    m_rows.reserve(static_cast<size_t>(count));
    for (int64_t i = 0; i < count; ++i)
        m_rows.push_back(i);

    if (m_sortedColumn != static_cast<size_t>(-1) && m_sortDirection != SortDirection::None)
        SortRows();

    m_selection.SetItemCount(static_cast<int64_t>(m_rows.size()));
}

void UIGridView::ClampScroll() noexcept
{
    const float totalW = GetTotalColumnWidth();
    const float maxX = (std::max)(0.0f, totalW - (GetWidth() - ScrollBarSize));
    m_scrollX = (std::clamp)(m_scrollX, 0.0f, maxX);

    const float headerOffset = m_headerVisible ? m_headerHeight : 0.0f;
    const float totalH = GetRowCount() * m_rowHeight + headerOffset;
    const float maxY = (std::max)(0.0f, totalH - (GetHeight() - ScrollBarSize));
    m_scrollY = (std::clamp)(m_scrollY, 0.0f, maxY);
}

void UIGridView::EnsureVisible(int64_t displayRow) noexcept
{
    const float top = displayRow * m_rowHeight;
    const float bottom = top + m_rowHeight;
    const float viewportH = GetHeight() - ScrollBarSize - (m_headerVisible ? m_headerHeight : 0.0f);

    float newScroll = m_scrollY;
    if (top < m_scrollY)
        newScroll = top;
    else if (bottom > m_scrollY + viewportH)
        newScroll = bottom - viewportH;

    m_scrollY = (std::max)(0.0f, newScroll);
    InvalidateVisual();
}

void UIGridView::OnSelectionChanged(const SelectionManager& sm) noexcept
{
    if (m_onSelectionChanged)
        m_onSelectionChanged(sm);
    InvalidateVisual();
}

float UIGridView::GetColumnX(size_t column) const noexcept
{
    float x = 0.0f;
    for (size_t i = 0; i < column && i < m_columns.size(); ++i)
        x += m_columns[i].width;
    return x;
}

float UIGridView::GetTotalColumnWidth() const noexcept
{
    float w = 0.0f;
    for (const auto& c : m_columns)
        w += c.width;
    return w;
}

std::wstring UIGridView::GetCellText(int64_t displayRow, size_t column) const noexcept
{
    const std::any value = FetchRow(displayRow);
    return GetCellText(value, column);
}

std::wstring UIGridView::GetCellText(const std::any& value, size_t column) const noexcept
{
    if (column < m_columns.size() && m_columns[column].getText)
        return m_columns[column].getText(value);
    if (const auto* s = std::any_cast<std::wstring>(&value))
        return *s;
    return L"";
}

std::any UIGridView::FetchRow(int64_t displayRow) const noexcept
{
    if (!m_source)
        return {};
    if (displayRow < 0 || displayRow >= static_cast<int64_t>(m_rows.size()))
        return {};
    return m_source->GetItem(m_rows[static_cast<size_t>(displayRow)]);
}

} // namespace DragonOS::DragonUI
