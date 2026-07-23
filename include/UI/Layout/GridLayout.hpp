#pragma once
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UILayout.hpp>
#include <vector>
#include <unordered_map>

namespace DragonOS::UI {

class GridLayout : public UIContainer {
public:
    GridLayout() noexcept = default;

    void AddColumn(const GridDefinition& col) noexcept;
    void AddRow(const GridDefinition& row) noexcept;
    void SetColumns(const std::vector<GridDefinition>& cols) noexcept;
    void SetRows(const std::vector<GridDefinition>& rows) noexcept;

    void SetChildPosition(UIElement* child, size_t col, size_t row) noexcept;

    void ClearDefinitions() noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    struct CellPosition { size_t col{0}, row{0}; };

    void ResolveStarSizes(float availableWidth, float availableHeight) noexcept;

    std::vector<GridDefinition> m_columns;
    std::vector<GridDefinition> m_rows;
    std::unordered_map<uint64_t, CellPosition> m_childCells;
    std::vector<float> m_columnWidths;
    std::vector<float> m_rowHeights;
};

} // namespace
