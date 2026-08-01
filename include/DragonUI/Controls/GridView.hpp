#pragma once

#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/SelectionManager.hpp>
#include <DragonUI/Core/VirtualItemSource.hpp>
#include <DragonUI/Controls/ListView.hpp> // Reuses SortDirection.
#include <Theme/ThemePalette.hpp>

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  Virtualized data grid with columns, headers, resizable
 *         columns, per-column sorting, selection and alternating row
 *         colours.
 *
 * Rows come from a VirtualItemSource so only the visible rows are ever
 * materialized.  Columns are addressed by index and may be resized by
 * dragging their header dividers.
 */
class UIGridView final : public Control {
public:
    using Comparer = std::function<int(const std::any&, const std::any&)>;
    using CellTextProvider = std::function<std::wstring(const std::any&)>;
    using ActivationCallback = std::function<void(UIGridView&, int64_t row, size_t column)>;

    struct Column {
        std::wstring title;
        float width{ 120.0f };
        CellTextProvider getText;  // Optional; falls back to any_cast<std::wstring>.
        std::function<Theme::SemanticColor(const std::any&)> getColor; // Optional cell colour.
        Comparer comparer;         // Optional per-column comparator.
        bool sortable{ true };
    };

    UIGridView() noexcept;

    // ── Data ─────────────────────────────────────────────────────────

    void SetRowSource(std::shared_ptr<VirtualItemSource> source) noexcept;
    [[nodiscard]] VirtualItemSource* GetRowSource() const noexcept { return m_source.get(); }

    void AddColumn(Column column) noexcept;
    void ClearColumns() noexcept;
    [[nodiscard]] const std::vector<Column>& GetColumns() const noexcept { return m_columns; }
    [[nodiscard]] size_t GetColumnCount() const noexcept { return m_columns.size(); }

    void SetRowHeight(float height) noexcept;
    [[nodiscard]] float GetRowHeight() const noexcept { return m_rowHeight; }
    void SetHeaderVisible(bool visible) noexcept;
    void SetAlternatingRowColors(bool enabled) noexcept;
    [[nodiscard]] bool GetAlternatingRowColors() const noexcept { return m_alternatingRowColors; }

    // ── Sorting ──────────────────────────────────────────────────────

    void SetSortColumn(size_t column, SortDirection direction) noexcept;
    void SortRows() noexcept;
    [[nodiscard]] size_t GetSortedColumn() const noexcept { return m_sortedColumn; }
    [[nodiscard]] SortDirection GetSortDirection() const noexcept { return m_sortDirection; }

    // ── Selection / activation ───────────────────────────────────────

    [[nodiscard]] SelectionManager& GetSelection() noexcept { return m_selection; }
    [[nodiscard]] const SelectionManager& GetSelection() const noexcept { return m_selection; }
    void SetOnSelectionChanged(std::function<void(const SelectionManager&)> cb) noexcept { m_onSelectionChanged = std::move(cb); }
    void SetOnCellActivated(ActivationCallback cb) noexcept { m_onCellActivated = std::move(cb); }

    [[nodiscard]] std::any GetRowItem(int64_t displayRow) const noexcept;
    [[nodiscard]] int64_t GetRowCount() const noexcept { return static_cast<int64_t>(m_rows.size()); }

    void Refresh() noexcept;

    // ── Scrolling ────────────────────────────────────────────────────

    void SetScrollOffset(float x, float y) noexcept;
    [[nodiscard]] float GetScrollX() const noexcept { return m_scrollX; }
    [[nodiscard]] float GetScrollY() const noexcept { return m_scrollY; }
    void ScrollToRow(int64_t displayRow) noexcept;

    // ── Hit testing ──────────────────────────────────────────────────

    [[nodiscard]] int64_t HitTestRowAt(float x, float y) const noexcept;
    [[nodiscard]] size_t HitTestColumnAt(float x) const noexcept;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;
    bool OnFocusEvent(const FocusEventArgs& args) noexcept override;

private:
    void SortIndices() noexcept;
    void RebuildRows() noexcept;
    void ClampScroll() noexcept;
    void EnsureVisible(int64_t displayRow) noexcept;
    void OnSelectionChanged(const SelectionManager& sm) noexcept;

    void RenderHeader(RenderContext& ctx, float contentX, float contentY) noexcept;
    void RenderRow(RenderContext& ctx, float contentX, float contentY, int64_t displayRow, float rowTop) noexcept;
    void RenderScrollBars(RenderContext& ctx) noexcept;

    [[nodiscard]] float GetColumnX(size_t column) const noexcept;
    [[nodiscard]] float GetTotalColumnWidth() const noexcept;
    [[nodiscard]] std::wstring GetCellText(int64_t displayRow, size_t column) const noexcept;
    [[nodiscard]] std::wstring GetCellText(const std::any& value, size_t column) const noexcept;
    [[nodiscard]] std::any FetchRow(int64_t displayRow) const noexcept;

    std::shared_ptr<VirtualItemSource> m_source;
    std::vector<int64_t> m_rows;  // Display order → source index.

    std::vector<Column> m_columns;
    size_t m_sortedColumn{ static_cast<size_t>(-1) };
    SortDirection m_sortDirection{ SortDirection::None };

    SelectionManager m_selection;
    std::function<void(const SelectionManager&)> m_onSelectionChanged;
    ActivationCallback m_onCellActivated;

    float m_rowHeight{ 26.0f };
    float m_headerHeight{ 26.0f };
    bool m_headerVisible{ true };
    bool m_alternatingRowColors{ true };

    float m_scrollX{};
    float m_scrollY{};
    int64_t m_hoveredRow{ -1 };
    size_t m_hoveredColumn{ static_cast<size_t>(-1) };
    int64_t m_dragColumn{ -1 };
    bool m_columnResizing{};

    static constexpr float ScrollBarSize = 12.0f;
    static constexpr float MinColumnWidth = 40.0f;
};

} // namespace DragonOS::DragonUI
