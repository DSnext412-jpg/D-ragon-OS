#pragma once
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UILayout.hpp>

namespace DragonOS::UI {

class StackPanel : public UIContainer {
public:
    explicit StackPanel(Orientation orientation = Orientation::Vertical) noexcept;

    void SetOrientation(Orientation orient) noexcept;
    [[nodiscard]] Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetSpacing(float spacing) noexcept { m_spacing = spacing; InvalidateLayout(); }
    [[nodiscard]] float GetSpacing() const noexcept { return m_spacing; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    Orientation m_orientation;
    float m_spacing{0};
};

} // namespace
