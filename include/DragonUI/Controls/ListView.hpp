#pragma once

#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/SelectionManager.hpp>
#include <DragonUI/Core/VirtualItemSource.hpp>
#include <DragonUI/Core/VirtualViewport.hpp>
#include <DragonUI/Core/ItemTemplate.hpp>

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

enum class ListViewMode : uint8_t {
    Details, //< Column headers + rows.
    List,    //< Simple one-column list with icons.
    Tile,    //< Grid of icon tiles with labels.
};

enum class SortDirection : uint8_t {
    None,
    Ascending,
    Descending,
};

/**
 * @brief  Virtualized list control supporting Details / List / Tile
 *         modes, single & multi selection, sorting, filtering, icons,
 *         keyboard navigation and item templates.
 *
 * The control renders only the rows that intersect its viewport, so it
 * stays fast with tens of thousands of items.  Selection, sorting and
 * filtering are delegated to reusable components.
 */
class UIListView final : public Control {
public:
    using TextProvider = std::function<std::wstring(const std::any&)>;
    using IconProvider = std::function<uint32_t(const std::any&)>;
    using Comparer = std::function<int(const std::any&, const std::any&)>;
    using FilterPredicate = std::function<bool(const std::any&)>;
    using ItemRenderFn = std::function<void(RenderContext&, const LayoutSlot&, const std::any&, const ItemVisualState&)>;
    using ActivationCallback = std::function<void(UIListView&, int64_t index)>;

    struct Column {
        std::wstring title;
        float width{ 140.0f };
        TextProvider getText;   // Optional; falls back to primary text.
        Comparer comparer;      // Optional per-column comparator.
        bool sortable{ false };
    };

    UIListView() noexcept;

    // ── Data source ──────────────────────────────────────────────────

    void SetItemSource(std::shared_ptr<VirtualItemSource> source) noexcept;
    [[nodiscard]] VirtualItemSource* GetItemSource() const noexcept { return m_source.get(); }

    void SetPrimaryTextProvider(TextProvider provider) noexcept;
    void SetPrimaryIconProvider(IconProvider provider) noexcept;
    void SetItemRenderer(ItemRenderFn fn) noexcept;
    void SetOnItemActivated(ActivationCallback cb) noexcept;
    void SetOnSelectionChanged(std::function<void(const SelectionManager&)> cb) noexcept;

    // ── Columns (Details mode) ───────────────────────────────────────

    void AddColumn(Column column) noexcept;
    void ClearColumns() noexcept;
    [[nodiscard]] const std::vector<Column>& GetColumns() const noexcept { return m_columns; }

    // ── Mode & layout ────────────────────────────────────────────────

    void SetMode(ListViewMode mode) noexcept;
    [[nodiscard]] ListViewMode GetMode() const noexcept { return m_mode; }
    void SetItemHeight(float height) noexcept;
    [[nodiscard]] float GetItemHeight() const noexcept { return m_itemHeight; }
    void SetTileExtent(float width, float height) noexcept;
    void SetHeaderVisible(bool visible) noexcept;

    // ── Sorting / filtering ──────────────────────────────────────────

    void SortByColumn(size_t columnIndex, SortDirection direction) noexcept;
    void SetGlobalSortComparer(Comparer cmp) noexcept;
    void SortItems() noexcept;
    void SetFilter(FilterPredicate pred) noexcept;
    void ClearFilter() noexcept;
    void Refresh() noexcept;

    // ── Selection ────────────────────────────────────────────────────

    [[nodiscard]] SelectionManager& GetSelection() noexcept { return m_selection; }
    [[nodiscard]] const SelectionManager& GetSelection() const noexcept { return m_selection; }
    [[nodiscard]] std::any GetItemAt(int64_t displayIndex) const noexcept;
    [[nodiscard]] int64_t GetDisplayCount() const noexcept { return static_cast<int64_t>(m_indices.size()); }

    // ── Scrolling ────────────────────────────────────────────────────

    void SetScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetScrollOffset() const noexcept { return m_viewport.GetScrollY(); }
    void ScrollToItem(int64_t displayIndex) noexcept;
    [[nodiscard]] float GetContentHeight() const noexcept;

    // ── Hit testing ──────────────────────────────────────────────────

    [[nodiscard]] int64_t HitTestItemAt(float x, float y) const noexcept;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    bool OnMouseEvent(EventType type, const MouseEventArgs& args) noexcept override;
    bool OnKeyEvent(EventType type, const KeyEventArgs& args) noexcept override;
    bool OnFocusEvent(const FocusEventArgs& args) noexcept override;

private:
    void RebuildDisplayList() noexcept;
    void UpdateViewport() noexcept;
    void OnSelectionChanged(const SelectionManager& sm) noexcept;
    void ScrollBy(float dy) noexcept;
    void EnsureVisible(int64_t displayIndex) noexcept;

    void RenderListMode(RenderContext& ctx, float contentX, float contentY) noexcept;
    void RenderDetailsMode(RenderContext& ctx, float contentX, float contentY) noexcept;
    void RenderTileMode(RenderContext& ctx, float contentX, float contentY) noexcept;
    void RenderHeader(RenderContext& ctx, float contentX, float contentY) noexcept;
    void RenderScrollBar(RenderContext& ctx) noexcept;

    [[nodiscard]] float GetColumnX(size_t column) const noexcept;
    [[nodiscard]] float GetColumnsTotalWidth() const noexcept;
    [[nodiscard]] std::wstring GetItemText(int64_t displayIndex, size_t column) const noexcept;
    [[nodiscard]] std::wstring GetItemText(const std::any& value, size_t column) const noexcept;
    [[nodiscard]] uint32_t GetItemIcon(int64_t displayIndex) const noexcept;
    [[nodiscard]] std::any FetchItem(int64_t displayIndex) const noexcept;
    [[nodiscard]] int64_t DisplayIndexFromSource(int64_t sourceIndex) const noexcept;

    // ── Data ─────────────────────────────────────────────────────────

    std::shared_ptr<VirtualItemSource> m_source;
    std::vector<int64_t> m_indices;      // Display order → source index.
    TextProvider m_primaryText;
    IconProvider m_primaryIcon;
    ItemRenderFn m_itemRenderer;
    ActivationCallback m_onActivated;
    std::function<void(const SelectionManager&)> m_onSelectionChanged;

    // ── Presentation ─────────────────────────────────────────────────

    std::vector<Column> m_columns;
    ListViewMode m_mode{ ListViewMode::List };
    float m_itemHeight{ 28.0f };
    float m_tileWidth{ 96.0f };
    float m_tileHeight{ 84.0f };
    bool m_headerVisible{ true };
    float m_headerHeight{ 26.0f };

    // ── Sorting / filtering ──────────────────────────────────────────

    Comparer m_globalComparer;
    FilterPredicate m_filter;
    size_t m_sortedColumn{ static_cast<size_t>(-1) };
    SortDirection m_sortDirection{ SortDirection::None };

    // ── Interaction state ────────────────────────────────────────────

    SelectionManager m_selection;
    VirtualViewport m_viewport;
    int64_t m_hoveredIndex{ -1 };
    int64_t m_dragColumn{ -1 };
    bool m_columnResizing{};

    static constexpr float ScrollBarWidth = 10.0f;
};

} // namespace DragonOS::DragonUI
