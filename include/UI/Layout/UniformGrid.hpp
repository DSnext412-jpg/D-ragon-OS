#pragma once
#include <UI/Core/UIContainer.hpp>

namespace DragonOS::UI {

class UniformGrid : public UIContainer {
public:
    UniformGrid() noexcept = default;

    void SetColumns(size_t cols) noexcept { m_columns = cols; InvalidateLayout(); }
    [[nodiscard]] size_t GetColumns() const noexcept { return m_columns; }
    void SetRows(size_t rows) noexcept { m_rows = rows; InvalidateLayout(); }
    [[nodiscard]] size_t GetRows() const noexcept { return m_rows; }

    void SetFirstColumn(size_t col) noexcept { m_firstColumn = col; InvalidateLayout(); }
    [[nodiscard]] size_t GetFirstColumn() const noexcept { return m_firstColumn; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    size_t m_columns{1};
    size_t m_rows{0};
    size_t m_firstColumn{0};
};

} // namespace
