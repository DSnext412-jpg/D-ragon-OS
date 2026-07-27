#pragma once

#include <DragonUI/Core/Container.hpp>

namespace DragonOS::DragonUI {

class UIStackPanel final : public Container {
public:
    UIStackPanel() noexcept = default;
    explicit UIStackPanel(Orientation orient) noexcept;

    void SetOrientation(Orientation orient) noexcept;
    [[nodiscard]] Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetSpacing(float spacing) noexcept;
    [[nodiscard]] float GetSpacing() const noexcept { return m_spacing; }

    void SetHorizontalAlignment(Alignment align) noexcept;
    [[nodiscard]] Alignment GetHorizontalAlignment() const noexcept { return m_hAlign; }
    void SetVerticalAlignment(Alignment align) noexcept;
    [[nodiscard]] Alignment GetVerticalAlignment() const noexcept { return m_vAlign; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    Orientation m_orientation{Orientation::Vertical};
    float m_spacing{};
    Alignment m_hAlign{Alignment::Stretch};
    Alignment m_vAlign{Alignment::Start};
};

} // namespace
