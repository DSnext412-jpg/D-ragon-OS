#pragma once

#include <DragonUI/Core/Container.hpp>
#include <unordered_map>

namespace DragonOS::DragonUI {

class UICanvas final : public Container {
public:
    UICanvas() noexcept = default;

    void SetChildPosition(Element& child, float left, float top);
    [[nodiscard]] std::pair<float, float> GetChildPosition(const Element& child) const;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    std::unordered_map<const Element*, std::pair<float, float>> m_positions;
};

} // namespace
