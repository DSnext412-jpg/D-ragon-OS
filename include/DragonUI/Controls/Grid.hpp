#pragma once

#include <DragonUI/Core/Container.hpp>
#include <vector>
#include <cstdint>

namespace DragonOS::DragonUI {

struct GridLength {
    enum class Type { Auto, Pixel, Star };
    Type type{Type::Auto};
    float value{};

    static constexpr GridLength Auto() noexcept { return {Type::Auto, 0.0f}; }
    static constexpr GridLength Pixel(float v) noexcept { return {Type::Pixel, v}; }
    static constexpr GridLength Star(float v = 1.0f) noexcept { return {Type::Star, v}; }

    bool operator==(const GridLength&) const noexcept = default;
};

struct RowDefinition {
    GridLength height{GridLength::Auto()};
    float minHeight{};
    float maxHeight{FLT_MAX};
    float actualOffset{};
    float actualHeight{};
};

struct ColumnDefinition {
    GridLength width{GridLength::Auto()};
    float minWidth{};
    float maxWidth{FLT_MAX};
    float actualOffset{};
    float actualWidth{};
};

class UIGrid final : public Container {
public:
    UIGrid() noexcept = default;

    void AddRow(GridLength height = GridLength::Auto(), float minH = {}, float maxH = FLT_MAX);
    void AddColumn(GridLength width = GridLength::Auto(), float minW = {}, float maxW = FLT_MAX);
    void RemoveAllRows() noexcept;
    void RemoveAllColumns() noexcept;

    [[nodiscard]] const std::vector<RowDefinition>& GetRows() const noexcept { return m_rows; }
    [[nodiscard]] const std::vector<ColumnDefinition>& GetColumns() const noexcept { return m_cols; }

    void SetChildPosition(Element& child, uint32_t row, uint32_t col);
    [[nodiscard]] std::pair<uint32_t, uint32_t> GetChildPosition(const Element& child) const;

    void SetRowSpacing(float spacing) noexcept;
    [[nodiscard]] float GetRowSpacing() const noexcept { return m_rowSpacing; }
    void SetColumnSpacing(float spacing) noexcept;
    [[nodiscard]] float GetColumnSpacing() const noexcept { return m_colSpacing; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    struct ChildGridInfo {
        Element* element{};
        uint32_t row{};
        uint32_t col{};
    };

    std::vector<RowDefinition> m_rows;
    std::vector<ColumnDefinition> m_cols;
    std::vector<ChildGridInfo> m_childGridInfo;
    float m_rowSpacing{};
    float m_colSpacing{};

    void MeasureAutoRows(const LayoutSlot& slot, std::vector<float>& rowHeights);
    void MeasureAutoCols(const LayoutSlot& slot, std::vector<float>& colWidths);
    void DistributeStarRows(float availableHeight, std::vector<float>& rowHeights);
    void DistributeStarCols(float availableWidth, std::vector<float>& colWidths);
};

inline constexpr auto& GridLengthAuto = GridLength::Auto;
inline constexpr auto& GridLengthPixel = GridLength::Pixel;
inline constexpr auto& GridLengthStar = GridLength::Star;

} // namespace
