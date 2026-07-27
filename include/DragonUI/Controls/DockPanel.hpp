#pragma once

#include <DragonUI/Core/Container.hpp>
#include <unordered_map>

namespace DragonOS::DragonUI {

enum class Dock { Left, Top, Right, Bottom, Fill };

class UIDockPanel final : public Container {
public:
    UIDockPanel() noexcept = default;

    void SetLastChildFill(bool fill) noexcept { m_lastChildFill = fill; }
    [[nodiscard]] bool GetLastChildFill() const noexcept { return m_lastChildFill; }

    void SetChildDock(Element& child, Dock dock);
    [[nodiscard]] Dock GetChildDock(const Element& child) const;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    bool m_lastChildFill{true};
    std::unordered_map<const Element*, Dock> m_dockMap;
};

} // namespace
