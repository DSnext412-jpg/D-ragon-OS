#pragma once
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UILayout.hpp>
#include <unordered_map>

namespace DragonOS::UI {

class DockPanel : public UIContainer {
public:
    DockPanel() noexcept = default;

    void SetLastChildFill(bool fill) noexcept { m_lastChildFill = fill; InvalidateLayout(); }
    [[nodiscard]] bool GetLastChildFill() const noexcept { return m_lastChildFill; }

    void SetChildDock(UIElement* child, DockPosition dock) noexcept;

    D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& available) noexcept override;
    void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept override;

protected:
    D2D1_RECT_F MeasureChildren(const D2D1_RECT_F&) noexcept override { return {0,0,0,0}; }
    void ArrangeChildren(const D2D1_RECT_F&) noexcept override {}

private:
    bool m_lastChildFill{true};
    std::unordered_map<uint64_t, DockPosition> m_childDock;
};

} // namespace
