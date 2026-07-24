#pragma once

#include <DragonUI/Core/Element.hpp>

namespace DragonOS::DragonUI {

class UISeparator final : public Element {
public:
    UISeparator() noexcept = default;

    void SetOrientation(Orientation orient) noexcept { m_orientation = orient; InvalidateLayout(); }
    [[nodiscard]] Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetThickness(float thickness) noexcept { m_thickness = thickness; InvalidateLayout(); }
    [[nodiscard]] float GetThickness() const noexcept { return m_thickness; }

    void SetColor(Theme::SemanticColor color) noexcept { m_color = color; InvalidateVisual(); }
    [[nodiscard]] Theme::SemanticColor GetColor() const noexcept { return m_color; }

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void Render(RenderContext& ctx) noexcept override;

private:
    Orientation m_orientation{Orientation::Horizontal};
    float m_thickness{1.0f};
    Theme::SemanticColor m_color{Theme::SemanticColor::WindowBorder};
};

} // namespace
