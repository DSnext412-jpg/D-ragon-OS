#pragma once
#include <UI/Core/UIContainer.hpp>
#include <string>
#include <vector>
#include <functional>

namespace DragonOS::UI {

struct GridViewColumn {
    std::wstring header;
    float width{80.0f};
};

class GridView : public UIContainer {
public:
    GridView() noexcept;

    void SetColumns(const std::vector<GridViewColumn>& columns) noexcept;
    [[nodiscard]] const std::vector<GridViewColumn>& GetColumns() const noexcept { return m_columns; }

    void SetRows(const std::vector<std::vector<std::wstring>>& rows) noexcept;
    [[nodiscard]] const std::vector<std::vector<std::wstring>>& GetRows() const noexcept { return m_rows; }

    void SetSelectedRow(int index) noexcept;
    [[nodiscard]] int GetSelectedRow() const noexcept { return m_selectedRow; }

    void SetOnSelectionChanged(std::function<void(int)> callback) noexcept { m_onSelectionChanged = std::move(callback); }

    void SetScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetScrollOffset() const noexcept { return m_scrollOffset; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float HeaderHeight = 28.0f;
    static constexpr float RowHeight = 24.0f;

private:
    std::vector<GridViewColumn> m_columns;
    std::vector<std::vector<std::wstring>> m_rows;
    int m_selectedRow{-1};
    int m_hoveredRow{-1};
    float m_scrollOffset{0};
    std::function<void(int)> m_onSelectionChanged;
};

} // namespace
