#pragma once

#include <cstdint>

namespace DragonOS::DragonUI {

/**
 * @brief  A pure math viewport used to determine which items of a very
 *         large list/grid must be rendered.
 *
 * Given the viewport size, the fixed item extent and the current scroll
 * offset, the viewport reports the first/last visible row and column
 * indices.  No item storage is involved — this is what allows smooth
 * scrolling over tens of thousands of rows while rendering a handful.
 */
class VirtualViewport final {
public:
    VirtualViewport() noexcept = default;

    void SetViewportSize(float width, float height) noexcept;
    void SetItemExtent(float width, float height) noexcept;
    void SetTotalRows(int64_t rows) noexcept;
    void SetTotalColumns(int64_t columns) noexcept;

    void SetScrollX(float x) noexcept;
    void SetScrollY(float y) noexcept;
    [[nodiscard]] float GetScrollX() const noexcept { return m_scrollX; }
    [[nodiscard]] float GetScrollY() const noexcept { return m_scrollY; }

    // ── Row addressing ───────────────────────────────────────────────

    [[nodiscard]] int64_t GetFirstVisibleRow() const noexcept;
    [[nodiscard]] int64_t GetLastVisibleRow() const noexcept;
    [[nodiscard]] int64_t GetVisibleRowCount() const noexcept;
    [[nodiscard]] float GetRowY(int64_t row) const noexcept;

    // ── Column addressing ────────────────────────────────────────────

    [[nodiscard]] int64_t GetFirstVisibleColumn() const noexcept;
    [[nodiscard]] int64_t GetLastVisibleColumn() const noexcept;
    [[nodiscard]] int64_t GetVisibleColumnCount() const noexcept;
    [[nodiscard]] float GetColumnX(int64_t column) const noexcept;

    // ── Content extent (scrollable bounds) ───────────────────────────

    [[nodiscard]] float GetTotalContentHeight() const noexcept;
    [[nodiscard]] float GetTotalContentWidth() const noexcept;
    [[nodiscard]] float GetMaxScrollY() const noexcept;
    [[nodiscard]] float GetMaxScrollX() const noexcept;

    [[nodiscard]] int64_t GetColumns() const noexcept;
    void SetColumns(int64_t columns) noexcept;

    void ClampScroll() noexcept;
    void Reset() noexcept;

private:
    float m_viewportWidth{};
    float m_viewportHeight{};
    float m_itemWidth{32.0f};
    float m_itemHeight{28.0f};
    float m_scrollX{};
    float m_scrollY{};
    int64_t m_totalRows{};
    int64_t m_columns{1};
};

} // namespace DragonOS::DragonUI
