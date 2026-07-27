#pragma once

#include <DragonUI/Core/Container.hpp>

namespace DragonOS::DragonUI {

class UIWrapPanel final : public Container {
public:
    UIWrapPanel() noexcept = default;
    explicit UIWrapPanel(Orientation orient) noexcept;

    void SetOrientation(Orientation orient) noexcept;
    [[nodiscard]] Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetItemWidth(float w) noexcept;
    [[nodiscard]] float GetItemWidth() const noexcept { return m_itemWidth; }
    void SetItemHeight(float h) noexcept;
    [[nodiscard]] float GetItemHeight() const noexcept { return m_itemHeight; }

    void SetHorizontalSpacing(float s) noexcept;
    [[nodiscard]] float GetHorizontalSpacing() const noexcept { return m_hSpacing; }
    void SetVerticalSpacing(float s) noexcept;
    [[nodiscard]] float GetVerticalSpacing() const noexcept { return m_vSpacing; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    Orientation m_orientation{Orientation::Horizontal};
    float m_itemWidth{};
    float m_itemHeight{};
    float m_hSpacing{};
    float m_vSpacing{};
};

} // namespace
