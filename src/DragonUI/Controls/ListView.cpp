#include <DragonUI/Controls/ListView.hpp>
#include <DragonUI/Core/Glyph.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <string>

namespace DragonOS::DragonUI {

namespace {
std::wstring ToLower(std::wstring_view text)
{
    std::wstring result(text);
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return result;
}

int CompareText(const std::wstring& a, const std::wstring& b)
{
    const std::wstring la = ToLower(a);
    const std::wstring lb = ToLower(b);
    const int cmp = la.compare(lb);
    return cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);
}
} // namespace

UIListView::UIListView() noexcept
{
    m_focusable = true;
    SetMinSize(160.0f, 120.0f);
    m_selection.SetMode(SelectionMode::Multi);
    m_selection.SetOnChanged([this](const SelectionManager& sm) { OnSelectionChanged(sm); });
}

// ── Data source ───────────────────────────────────────────────────────

void UIListView::SetItemSource(std::shared_ptr<VirtualItemSource> source) noexcept
{
    m_source = std::move(source);
    m_selection.Clear();
    RebuildDisplayList();
    InvalidateLayout();
}

void UIListView::SetPrimaryTextProvider(TextProvider provider) noexcept
{
    m_primaryText = std::move(provider);
    InvalidateVisual();
}

void UIListView::SetPrimaryIconProvider(IconProvider provider) noexcept
{
    m_primaryIcon = std::move(provider);
    InvalidateVisual();
}

void UIListView::SetItemRenderer(ItemRenderFn fn) noexcept
{
    m_itemRenderer = std::move(fn);
    InvalidateVisual();
}

void UIListView::SetOnItemActivated(ActivationCallback cb) noexcept
{
    m_onActivated = std::move(cb);
}

void UIListView::SetOnSelectionChanged(std::function<void(const SelectionManager&)> cb) noexcept
{
    m_onSelectionChanged = std::move(cb);
}

// ── Columns ───────────────────────────────────────────────────────────

void UIListView::AddColumn(Column column) noexcept
{
    m_columns.push_back(std::move(column));
    InvalidateVisual();
}

void UIListView::ClearColumns() noexcept
{
    m_columns.clear();
    InvalidateVisual();
}

// ── Mode & layout ─────────────────────────────────────────────────────

void UIListView::SetMode(ListViewMode mode) noexcept
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    m_viewport.Reset();
    UpdateViewport();
    InvalidateLayout();
}

void UIListView::SetItemHeight(float height) noexcept
{
    m_itemHeight = (std::max)(12.0f, height);
    UpdateViewport();
    InvalidateLayout();
}

void UIListView::SetTileExtent(float width, float height) noexcept
{
    m_tileWidth = (std::max)(32.0f, width);
    m_tileHeight = (std::max)(32.0f, height);
    UpdateViewport();
    InvalidateLayout();
}

void UIListView::SetHeaderVisible(bool visible) noexcept
{
    m_headerVisible = visible;
    InvalidateLayout();
}

// ── Sorting / filtering ───────────────────────────────────────────────

void UIListView::SortByColumn(size_t columnIndex, SortDirection direction) noexcept
{
    if (direction == SortDirection::None)
    {
        m_sortedColumn = static_cast<size_t>(-1);
        m_sortDirection = SortDirection::None;
        m_globalComparer = {};
        RebuildDisplayList();
        InvalidateVisual();
        return;
    }

    m_sortedColumn = columnIndex;
    m_sortDirection = direction;
    m_globalComparer = {};
    if (columnIndex < m_columns.size() && m_columns[columnIndex].comparer)
        m_globalComparer = m_columns[columnIndex].comparer;
    SortItems();
}

void UIListView::SetGlobalSortComparer(Comparer cmp) noexcept
{
    m_globalComparer = std::move(cmp);
    if (m_globalComparer)
        m_sortDirection = SortDirection::Ascending;
}

void UIListView::SortItems() noexcept
{
    if (m_indices.size() < 2)
        return;

    const Comparer cmp = m_globalComparer
        ? m_globalComparer
        : [this](const std::any& a, const std::any& b) {
              return CompareText(GetItemText(a, 0), GetItemText(b, 0));
          };

    std::stable_sort(m_indices.begin(), m_indices.end(), [this, &cmp](int64_t a, int64_t b) {
        const std::any va = m_source ? m_source->GetItem(a) : std::any{};
        const std::any vb = m_source ? m_source->GetItem(b) : std::any{};
        int result = cmp(va, vb);
        if (m_sortDirection == SortDirection::Descending)
            result = -result;
        return result < 0;
    });

    InvalidateVisual();
}

void UIListView::SetFilter(FilterPredicate pred) noexcept
{
    m_filter = std::move(pred);
    RebuildDisplayList();
    InvalidateVisual();
}

void UIListView::ClearFilter() noexcept
{
    if (m_filter)
    {
        m_filter = {};
        RebuildDisplayList();
        InvalidateVisual();
    }
}

void UIListView::Refresh() noexcept
{
    RebuildDisplayList();
    InvalidateVisual();
}

// ── Selection ─────────────────────────────────────────────────────────

std::any UIListView::GetItemAt(int64_t displayIndex) const noexcept
{
    return FetchItem(displayIndex);
}

// ── Scrolling ─────────────────────────────────────────────────────────

void UIListView::SetScrollOffset(float offset) noexcept
{
    m_viewport.SetScrollY(offset);
    InvalidateVisual();
}

void UIListView::ScrollToItem(int64_t displayIndex) noexcept
{
    EnsureVisible(displayIndex);
}

float UIListView::GetContentHeight() const noexcept
{
    return m_viewport.GetTotalContentHeight();
}

// ── Hit testing ───────────────────────────────────────────────────────

int64_t UIListView::HitTestItemAt(float x, float y) const noexcept
{
    const float localX = x - GetX();
    const float localY = y - GetY();

    const float contentWidth = GetWidth() - ScrollBarWidth;
    const bool inside = localX >= 0.0f && localX < contentWidth &&
                        localY >= 0.0f && localY <= GetHeight();
    if (!inside)
        return -1;

    float headerOffset = 0.0f;
    if (m_mode == ListViewMode::Details && m_headerVisible)
        headerOffset = m_headerHeight;

    if (localY < headerOffset)
        return -1;

    if (m_mode == ListViewMode::Tile)
    {
        const int64_t columns = (std::max<int64_t>)(1, m_viewport.GetColumns());
        const int64_t col = static_cast<int64_t>((localX + m_viewport.GetScrollX()) / m_tileWidth);
        const int64_t row = static_cast<int64_t>((localY - headerOffset + m_viewport.GetScrollY()) / m_tileHeight);
        if (col >= columns)
            return -1;
        const int64_t index = row * columns + col;
        return (index >= 0 && index < GetDisplayCount()) ? index : -1;
    }

    const int64_t row = static_cast<int64_t>((localY - headerOffset + m_viewport.GetScrollY()) / m_itemHeight);
    return (row >= 0 && row < GetDisplayCount()) ? row : -1;
}

// ── Layout ────────────────────────────────────────────────────────────

DesiredSize UIListView::MeasureOverride(const LayoutSlot& /*available*/) noexcept
{
    return { m_minW > 0.0f ? m_minW : 200.0f, m_minH > 0.0f ? m_minH : 200.0f };
}

// ── Rendering ─────────────────────────────────────────────────────────

void UIListView::Render(RenderContext& ctx) noexcept
{
    if (m_visibility != Visibility::Visible || m_opacity <= 0.0f)
        return;

    m_visualDirty = false;

    const float contentWidth = (std::max)(0.0f, GetWidth() - ScrollBarWidth);
    const float contentHeight = GetHeight();
    m_viewport.SetViewportSize(contentWidth, contentHeight);

    if (m_mode == ListViewMode::Tile)
    {
        const int64_t columns = (std::max<int64_t>)(1,
            static_cast<int64_t>((contentWidth + 4.0f) / (m_tileWidth + 4.0f)));
        m_viewport.SetTotalColumns(columns);
        const int64_t rows = columns > 0
            ? (GetDisplayCount() + columns - 1) / columns
            : 0;
        m_viewport.SetTotalRows(rows);
        m_viewport.SetItemExtent(m_tileWidth, m_tileHeight);
    }
    else
    {
        m_viewport.SetTotalColumns(1);
        m_viewport.SetTotalRows(GetDisplayCount());
        m_viewport.SetItemExtent(contentWidth, m_itemHeight);
    }

    m_viewport.ClampScroll();

    const auto d2d = static_cast<D2D1_RECT_F>(GetBounds());
    ctx.FillRectangle(d2d, Theme::SemanticColor::ControlFill, 0.4f);

    ctx.PushClip(d2d);

    const float contentX = GetX();
    const float contentY = GetY();

    switch (m_mode)
    {
    case ListViewMode::Details:
        if (m_headerVisible)
            RenderHeader(ctx, contentX, contentY);
        RenderDetailsMode(ctx, contentX, contentY);
        break;
    case ListViewMode::List:
        RenderListMode(ctx, contentX, contentY);
        break;
    case ListViewMode::Tile:
        RenderTileMode(ctx, contentX, contentY);
        break;
    }

    ctx.PopClip();

    RenderScrollBar(ctx);
}

void UIListView::RenderListMode(RenderContext& ctx, float contentX, float contentY) noexcept
{
    if (m_indices.empty())
    {
        ctx.DrawText(L"No items", { contentX + 12.0f, contentY + 8.0f, contentX + 200.0f, contentY + 28.0f },
            Theme::SemanticColor::PlaceholderText);
        return;
    }

    const int64_t first = m_viewport.GetFirstVisibleRow();
    const int64_t last = m_viewport.GetLastVisibleRow();
    const int64_t rowCount = last >= first ? last - first + 1 : 0;
    const float scrollY = m_viewport.GetScrollY();

    for (int64_t row = first; row <= last; ++row)
    {
        const float top = contentY + m_viewport.GetRowY(row) - scrollY;
        if (top + m_itemHeight < contentY || top > contentY + GetHeight())
            continue;

        const LayoutSlot slot{ contentX, top, GetWidth() - ScrollBarWidth, m_itemHeight };
        const int64_t index = row;

        const bool selected = m_selection.IsSelected(index);
        const bool hovered = (index == m_hoveredIndex);
        const bool focused = (GetControlState() == ControlState::Focused);

        if (selected)
            ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Selection);
        else if (hovered)
            ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover);

        const ItemVisualState state{ selected, hovered, focused, m_enabled };

        if (m_itemRenderer)
        {
            const auto value = FetchItem(index);
            m_itemRenderer(ctx, slot, value, state);
            continue;
        }

        const uint32_t icon = GetItemIcon(index);
        const std::wstring text = GetItemText(index, 0);

        float tx = slot.x + 8.0f;
        if (icon != 0)
        {
            const std::wstring glyph = CodepointToUtf16(icon);
            const auto gSize = ctx.MeasureText(glyph, 32.0f);
            const D2D1_RECT_F gRect = {
                tx,
                slot.y + (slot.height - gSize.height) * 0.5f,
                tx + gSize.width,
                slot.y + (slot.height - gSize.height) * 0.5f + gSize.height
            };
            ctx.DrawText(glyph, gRect, selected ? Theme::SemanticColor::TextPrimary
                                                : Theme::SemanticColor::TextSecondary);
            tx += gSize.width + 6.0f;
        }

        const D2D1_RECT_F textRect = {
            tx, slot.y + 2.0f,
            slot.x + slot.width - 8.0f, slot.y + slot.height - 2.0f
        };
        ctx.DrawText(text, textRect, Theme::SemanticColor::TextPrimary);
    }

    (void)rowCount;
}

void UIListView::RenderDetailsMode(RenderContext& ctx, float contentX, float contentY) noexcept
{
    if (m_indices.empty())
    {
        ctx.DrawText(L"No items", { contentX + 12.0f, contentY + m_headerHeight + 8.0f,
                                    contentX + 200.0f, contentY + m_headerHeight + 28.0f },
            Theme::SemanticColor::PlaceholderText);
        return;
    }

    const float headerOffset = m_headerVisible ? m_headerHeight : 0.0f;
    const float scrollY = m_viewport.GetScrollY();

    // Row range within the visible viewport (rows are indexed below the header).
    const int64_t totalRows = GetDisplayCount();
    if (totalRows == 0)
        return;

    const int64_t first = static_cast<int64_t>(scrollY / m_itemHeight);
    const int64_t last = static_cast<int64_t>((scrollY + (GetHeight() - headerOffset)) / m_itemHeight);

    for (int64_t row = first; row <= (std::min)(last, totalRows - 1); ++row)
    {
        const float top = contentY + headerOffset + m_viewport.GetRowY(row) - scrollY;
        if (top + m_itemHeight < contentY || top > contentY + GetHeight())
            continue;

        const int64_t index = row;
        const bool selected = m_selection.IsSelected(index);
        const bool hovered = (index == m_hoveredIndex);
        const bool focused = (GetControlState() == ControlState::Focused);

        const LayoutSlot slot{ contentX, top, GetWidth() - ScrollBarWidth, m_itemHeight };
        if (selected)
            ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Selection);
        else if (hovered)
            ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover);

        if (m_itemRenderer)
        {
            m_itemRenderer(ctx, slot, FetchItem(index), ItemVisualState{ selected, hovered, focused, m_enabled });
            continue;
        }

        for (size_t c = 0; c < m_columns.size(); ++c)
        {
            const float cx = contentX + GetColumnX(c);
            const std::wstring text = GetItemText(index, c);
            const D2D1_RECT_F cell = { cx + 8.0f, slot.y + 2.0f,
                                       cx + m_columns[c].width - 4.0f, slot.y + slot.height - 2.0f };
            ctx.DrawText(text, cell, Theme::SemanticColor::TextPrimary);
        }
    }
}

void UIListView::RenderTileMode(RenderContext& ctx, float contentX, float contentY) noexcept
{
    if (m_indices.empty())
    {
        ctx.DrawText(L"No items", { contentX + 12.0f, contentY + 8.0f, contentX + 200.0f, contentY + 28.0f },
            Theme::SemanticColor::PlaceholderText);
        return;
    }

    const int64_t columns = (std::max<int64_t>)(1, m_viewport.GetColumns());
    const int64_t firstRow = m_viewport.GetFirstVisibleRow();
    const int64_t lastRow = m_viewport.GetLastVisibleRow();
    const float scrollY = m_viewport.GetScrollY();

    for (int64_t row = firstRow; row <= lastRow; ++row)
    {
        for (int64_t col = 0; col < columns; ++col)
        {
            const int64_t index = row * columns + col;
            if (index >= GetDisplayCount())
                break;

            const float left = contentX + col * (m_tileWidth + 4.0f);
            const float top = contentY + row * (m_tileHeight + 4.0f) - scrollY;
            if (top + m_tileHeight < contentY || top > contentY + GetHeight())
                continue;

            const LayoutSlot slot{ left, top, m_tileWidth, m_tileHeight };
            const bool selected = m_selection.IsSelected(index);
            const bool hovered = (index == m_hoveredIndex);

            if (selected)
                ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Selection);
            else if (hovered)
                ctx.FillRectangle(static_cast<D2D1_RECT_F>(slot), Theme::SemanticColor::Hover);

            if (m_itemRenderer)
            {
                m_itemRenderer(ctx, slot, FetchItem(index),
                    ItemVisualState{ selected, hovered, GetControlState() == ControlState::Focused, m_enabled });
                continue;
            }

            const uint32_t icon = GetItemIcon(index);
            const float iconSize = 36.0f;
            const float iconX = left + (m_tileWidth - iconSize) * 0.5f;
            if (icon != 0)
            {
                const std::wstring glyph = CodepointToUtf16(icon);
                const D2D1_RECT_F iconRect = { iconX, top + 4.0f, iconX + iconSize, top + 4.0f + iconSize };
                ctx.DrawText(glyph, iconRect, Theme::SemanticColor::TextSecondary);
            }
            else
            {
                const D2D1_RECT_F box = { iconX, top + 4.0f, iconX + iconSize, top + 4.0f + iconSize };
                ctx.FillRoundedRect(box, Theme::SemanticColor::ControlBorder, 4.0f, 4.0f);
            }

            const std::wstring text = GetItemText(index, 0);
            const D2D1_RECT_F label = { left + 2.0f, top + iconSize + 6.0f,
                                        left + m_tileWidth - 2.0f, top + m_tileHeight - 2.0f };
            ctx.DrawText(text, label, Theme::SemanticColor::TextPrimary);
        }
    }
}

void UIListView::RenderHeader(RenderContext& ctx, float contentX, float contentY) noexcept
{
    const D2D1_RECT_F headerRect = {
        contentX, contentY,
        contentX + GetWidth() - ScrollBarWidth, contentY + m_headerHeight
    };
    ctx.FillRectangle(headerRect, Theme::SemanticColor::WindowTitleBar);

    for (size_t c = 0; c < m_columns.size(); ++c)
    {
        const float cx = contentX + GetColumnX(c);
        const float width = m_columns[c].width;

        const D2D1_RECT_F cell = { cx + 8.0f, contentY + 4.0f,
                                   cx + width - 4.0f, contentY + m_headerHeight - 4.0f };
        ctx.DrawText(m_columns[c].title, cell, Theme::SemanticColor::TextSecondary);

        if (c == m_sortedColumn && m_sortDirection != SortDirection::None)
        {
            std::wstring_view arrow = m_sortDirection == SortDirection::Ascending ? L"\x25B2" : L"\x25BC";
            const D2D1_RECT_F arrowRect = { cx + width - 18.0f, contentY + 4.0f,
                                            cx + width - 2.0f, contentY + m_headerHeight - 4.0f };
            ctx.DrawText(arrow, arrowRect, Theme::SemanticColor::Accent);
        }

        // Column divider (resize handle)
        if (c + 1 < m_columns.size())
        {
            const D2D1_RECT_F divider = { cx + width - 1.0f, contentY + 2.0f,
                                          cx + width + 1.0f, contentY + m_headerHeight - 2.0f };
            ctx.FillRectangle(divider, Theme::SemanticColor::MenuSeparator, 0.6f);
        }
    }
}

void UIListView::RenderScrollBar(RenderContext& ctx) noexcept
{
    const float maxScroll = m_viewport.GetMaxScrollY();
    if (maxScroll <= 0.0f)
        return;

    const float viewportH = GetHeight();
    const float contentH = viewportH + maxScroll;
    const float thumbSize = (std::max)(20.0f, (viewportH / contentH) * viewportH);
    const float thumbPos = (m_viewport.GetScrollY() / maxScroll) * (viewportH - thumbSize);

    const D2D1_RECT_F track = {
        GetX() + GetWidth() - ScrollBarWidth, GetY(),
        GetX() + GetWidth(), GetY() + viewportH
    };
    ctx.FillRectangle(track, Theme::SemanticColor::ControlFill, 0.5f);

    const D2D1_RECT_F thumb = {
        track.left + 1.0f, GetY() + thumbPos,
        track.right - 1.0f, GetY() + thumbPos + thumbSize
    };
    ctx.FillRoundedRect(thumb, Theme::SemanticColor::Accent, 3.0f, 3.0f, 0.85f);
}

// ── Mouse ─────────────────────────────────────────────────────────────

bool UIListView::OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept
{
    if (type == EventType::MouseMove && std::abs(args.wheelDelta) > 0.1f)
    {
        const float delta = args.wheelDelta > 0.0f ? -1.0f : 1.0f;
        ScrollBy(delta * m_itemHeight * 3.0f);
        return true;
    }

    const float localX = args.x - GetX();
    const float localY = args.y - GetY();

    switch (type)
    {
    case EventType::MouseMove:
        m_hoveredIndex = HitTestItemAt(args.x, args.y);
        if (m_columnResizing && m_dragColumn >= 0)
        {
            const int64_t col = m_dragColumn;
            if (col >= 0 && static_cast<size_t>(col) < m_columns.size())
            {
                const float newWidth = (std::max)(40.0f, localX - GetColumnX(static_cast<size_t>(col)));
                m_columns[static_cast<size_t>(col)].width = newWidth;
                InvalidateVisual();
            }
            return true;
        }
        InvalidateVisual();
        return true;

    case EventType::MouseDown:
        if (args.button == Input::MouseButton::Left)
        {
            if (m_mode == ListViewMode::Details && m_headerVisible && localY < m_headerHeight)
            {
                // Hit-test column dividers first.
                for (size_t c = 0; c + 1 < m_columns.size(); ++c)
                {
                    const float dividerX = GetColumnX(c) + m_columns[c].width;
                    if (std::abs(localX - dividerX) <= 4.0f)
                    {
                        m_columnResizing = true;
                        m_dragColumn = static_cast<int64_t>(c);
                        return true;
                    }
                }

                // Otherwise treat as a header sort click.
                float acc = 0.0f;
                for (size_t c = 0; c < m_columns.size(); ++c)
                {
                    acc += m_columns[c].width;
                    if (localX < acc)
                    {
                        const SortDirection dir =
                            m_sortedColumn == c && m_sortDirection == SortDirection::Ascending
                                ? SortDirection::Descending
                                : SortDirection::Ascending;
                        SortByColumn(c, dir);
                        return true;
                    }
                }
                return true;
            }

            const int64_t index = HitTestItemAt(args.x, args.y);
            if (index >= 0)
            {
                m_selection.OnPointerClick(index, args.ctrl, args.shift);
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
        const int64_t index = HitTestItemAt(args.x, args.y);
        if (index >= 0 && m_onActivated)
        {
            m_onActivated(*this, index);
            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

bool UIListView::OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept
{
    if (type != EventType::KeyDown)
        return false;

    const int64_t count = GetDisplayCount();
    if (count == 0)
        return false;

    int64_t current = m_selection.GetCurrent();

    auto move = [&](int64_t target) {
        target = (std::clamp)(target, int64_t{ 0 }, count - 1);
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
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_itemHeight) - 1);
        move(current < 0 ? 0 : current + page);
        return true;
    }

    case Input::KeyCode::PageUp:
    {
        const int64_t page = (std::max<int64_t>)(1, static_cast<int64_t>(GetHeight() / m_itemHeight) - 1);
        move(current < 0 ? 0 : current - page);
        return true;
    }

    case Input::KeyCode::Home:
        move(0);
        return true;

    case Input::KeyCode::End:
        move(count - 1);
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
        if (current >= 0 && m_onActivated)
        {
            m_onActivated(*this, current);
            return true;
        }
        return false;

    default:
        return false;
    }
}

bool UIListView::OnFocusEvent(const FocusEventArgs&) noexcept
{
    InvalidateVisual();
    return false;
}

// ── Private helpers ───────────────────────────────────────────────────

void UIListView::RebuildDisplayList() noexcept
{
    m_indices.clear();

    const int64_t count = m_source ? m_source->GetCount() : 0;
    m_indices.reserve(static_cast<size_t>(count));
    for (int64_t i = 0; i < count; ++i)
    {
        if (m_filter)
        {
            const std::any value = m_source->GetItem(i);
            if (!m_filter(value))
                continue;
        }
        m_indices.push_back(i);
    }

    if (m_globalComparer && m_sortDirection != SortDirection::None)
        SortItems();

    m_selection.SetItemCount(static_cast<int64_t>(m_indices.size()));
    UpdateViewport();
}

void UIListView::UpdateViewport() noexcept
{
    if (m_mode == ListViewMode::Tile)
    {
        const float contentWidth = (std::max)(0.0f, GetWidth() - ScrollBarWidth);
        const int64_t columns = (std::max<int64_t>)(1,
            static_cast<int64_t>((contentWidth + 4.0f) / (m_tileWidth + 4.0f)));
        m_viewport.SetTotalColumns(columns);
        const int64_t rows = columns > 0
            ? (GetDisplayCount() + columns - 1) / columns
            : 0;
        m_viewport.SetTotalRows(rows);
        m_viewport.SetItemExtent(m_tileWidth, m_tileHeight);
    }
    else
    {
        m_viewport.SetTotalColumns(1);
        m_viewport.SetTotalRows(GetDisplayCount());
        m_viewport.SetItemExtent((std::max)(1.0f, GetWidth() - ScrollBarWidth), m_itemHeight);
    }
    m_viewport.ClampScroll();
}

void UIListView::OnSelectionChanged(const SelectionManager& sm) noexcept
{
    if (m_onSelectionChanged)
        m_onSelectionChanged(sm);
    InvalidateVisual();
}

void UIListView::ScrollBy(float dy) noexcept
{
    m_viewport.SetScrollY(m_viewport.GetScrollY() + dy);
    InvalidateVisual();
}

void UIListView::EnsureVisible(int64_t displayIndex) noexcept
{
    if (displayIndex < 0 || displayIndex >= GetDisplayCount())
        return;

    const int64_t columns = (std::max<int64_t>)(1, m_viewport.GetColumns());
    const int64_t row = m_mode == ListViewMode::Tile ? displayIndex / columns : displayIndex;
    const float itemH = m_mode == ListViewMode::Tile ? m_tileHeight : m_itemHeight;

    const float top = m_viewport.GetRowY(row);
    const float bottom = top + itemH;
    const float scrollY = m_viewport.GetScrollY();
    const float viewportH = GetHeight();

    float newScroll = scrollY;
    if (top < scrollY)
        newScroll = top;
    else if (bottom > scrollY + viewportH)
        newScroll = bottom - viewportH;

    m_viewport.SetScrollY(newScroll);
    InvalidateVisual();
}

float UIListView::GetColumnX(size_t column) const noexcept
{
    float x = 0.0f;
    for (size_t i = 0; i < column && i < m_columns.size(); ++i)
        x += m_columns[i].width;
    return x;
}

float UIListView::GetColumnsTotalWidth() const noexcept
{
    float w = 0.0f;
    for (const auto& c : m_columns)
        w += c.width;
    return w;
}

std::wstring UIListView::GetItemText(int64_t displayIndex, size_t column) const noexcept
{
    const std::any value = FetchItem(displayIndex);
    return GetItemText(value, column);
}

std::wstring UIListView::GetItemText(const std::any& value, size_t column) const noexcept
{
    if (column < m_columns.size() && m_columns[column].getText)
        return m_columns[column].getText(value);
    if (m_primaryText)
        return m_primaryText(value);
    if (const auto* s = std::any_cast<std::wstring>(&value))
        return *s;
    return L"";
}

uint32_t UIListView::GetItemIcon(int64_t displayIndex) const noexcept
{
    if (!m_primaryIcon)
        return 0;
    const std::any value = FetchItem(displayIndex);
    return m_primaryIcon(value);
}

std::any UIListView::FetchItem(int64_t displayIndex) const noexcept
{
    if (!m_source)
        return {};
    if (displayIndex < 0 || displayIndex >= static_cast<int64_t>(m_indices.size()))
        return {};
    return m_source->GetItem(m_indices[static_cast<size_t>(displayIndex)]);
}

} // namespace DragonOS::DragonUI
