#pragma once

#include <DragonUI/Core/Element.hpp>
#include <vector>
#include <memory>

namespace DragonOS::DragonUI {

class Container : public Element {
public:
    Container() noexcept = default;

    virtual void AddChild(std::unique_ptr<Element> child) noexcept;
    virtual bool RemoveChild(Element* child) noexcept;
    void ClearChildren() noexcept;

    [[nodiscard]] Element* GetChildAt(size_t index) const noexcept;
    [[nodiscard]] size_t GetChildCount() const noexcept { return m_children.size(); }
    [[nodiscard]] const std::vector<std::unique_ptr<Element>>& GetChildren() const noexcept { return m_children; }

    void SetClipChildren(bool clip) noexcept { m_clipChildren = clip; }
    [[nodiscard]] bool GetClipChildren() const noexcept { return m_clipChildren; }

    void Measure(const LayoutSlot& available) noexcept override;
    void Arrange(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    [[nodiscard]] Element* HitTest(float x, float y) noexcept override;

    virtual void SortChildrenByZOrder() noexcept;

protected:
    virtual void MeasureChildren(const LayoutSlot& available) noexcept;
    virtual void ArrangeChildren(const LayoutSlot& finalSlot) noexcept;
    virtual void RenderChildren(RenderContext& ctx) noexcept;
    virtual Element* HitTestChildren(float x, float y) noexcept;

    std::vector<std::unique_ptr<Element>> m_children;
    bool m_clipChildren{false};
};

} // namespace
