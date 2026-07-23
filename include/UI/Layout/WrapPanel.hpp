#pragma once
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UILayout.hpp>

namespace DragonOS::UI {

class WrapPanel : public UIContainer {
public:
    explicit WrapPanel(Orientation orientation = Orientation::Horizontal) noexcept;

    void SetOrientation(Orientation orient) noexcept;
    [[nodiscard]] Orientation GetOrientation() const noexcept { return m_orientation; }

    void SetSpacing(float spacing) noexcept { m_spacing = spacing; InvalidateLayout(); }
    [[nodiscard]] float GetSpacing() const noexcept { return m_spacing; }

    void SetItemWidth(float w) noexcept { m_itemWidth = w; InvalidateLayout(); }
    [[nodiscard]] float GetItemWidth() const noexcept { return m_itemWidth; }
    void SetItemHeight(float h) noexcept { m_itemHeight = h; InvalidateLayout(); }
    [[nodiscard]] float GetItemHeight() const noexcept { return m_itemHeight; }

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    Orientation m_orientation{Orientation::Horizontal};
    float m_spacing{0};
    float m_itemWidth{0}, m_itemHeight{0};
};

} // namespace
