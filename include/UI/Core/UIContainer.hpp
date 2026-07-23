#pragma once
#include "UIElement.hpp"
#include <vector>
#include <algorithm>

namespace DragonOS::UI {

class UIContainer : public UIElement {
public:
    UIContainer() noexcept = default;

    void AddChild(std::unique_ptr<UIElement> child) noexcept;
    bool RemoveChild(UIElement* child) noexcept;
    void ClearChildren() noexcept;
    [[nodiscard]] UIElement* GetChildAt(size_t index) const noexcept;
    [[nodiscard]] size_t GetChildCount() const noexcept { return m_children.size(); }
    [[nodiscard]] const std::vector<std::unique_ptr<UIElement>>& GetChildren() const noexcept { return m_children; }

    void SetClipChildren(bool clip) noexcept { m_clipChildren = clip; }
    [[nodiscard]] bool GetClipChildren() const noexcept { return m_clipChildren; }

    void Measure(const D2D1_RECT_F& availableSize) noexcept override;
    void Arrange(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    [[nodiscard]] UIElement* HitTest(float x, float y) noexcept override;
    [[nodiscard]] bool OnEvent(const UIEvent& event) noexcept override;

    void SortChildrenByZIndex() noexcept;

protected:
    [[nodiscard]] virtual D2D1_RECT_F MeasureChildren(const D2D1_RECT_F& availableSize) noexcept;
    virtual void ArrangeChildren(const D2D1_RECT_F& finalRect) noexcept;

    std::vector<std::unique_ptr<UIElement>> m_children;
    bool m_clipChildren{false};
};

} // namespace
