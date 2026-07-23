#pragma once
#include <UI/Core/UIContainer.hpp>
#include <unordered_map>

namespace DragonOS::UI {

class Canvas : public UIContainer {
public:
    Canvas() noexcept = default;

    void SetChildPosition(UIElement* child, float x, float y) noexcept;
    [[nodiscard]] std::pair<float, float> GetChildPosition(const UIElement* child) const noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    std::unordered_map<uint64_t, D2D1_POINT_2F> m_positions;
};

} // namespace
