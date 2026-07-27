#pragma once

#include <DragonUI/Core/Container.hpp>

namespace DragonOS::DragonUI {

class UIScrollViewer final : public Container {
public:
    UIScrollViewer() noexcept = default;

    void SetHorizontalScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetHorizontalScrollOffset() const noexcept { return m_hScrollOffset; }
    void SetVerticalScrollOffset(float offset) noexcept;
    [[nodiscard]] float GetVerticalScrollOffset() const noexcept { return m_vScrollOffset; }

    void SetHorizontalScrollEnabled(bool enabled) noexcept;
    [[nodiscard]] bool IsHorizontalScrollEnabled() const noexcept { return m_hScrollEnabled; }
    void SetVerticalScrollEnabled(bool enabled) noexcept;
    [[nodiscard]] bool IsVerticalScrollEnabled() const noexcept { return m_vScrollEnabled; }

    [[nodiscard]] float GetScrollableWidth() const noexcept { return m_scrollableWidth; }
    [[nodiscard]] float GetScrollableHeight() const noexcept { return m_scrollableHeight; }

    void ScrollBy(float dx, float dy) noexcept;
    void ScrollTo(float x, float y) noexcept;

    DesiredSize MeasureOverride(const LayoutSlot& available) noexcept override;
    void ArrangeOverride(const LayoutSlot& finalSlot) noexcept override;
    void Render(RenderContext& ctx) noexcept override;
    Element* HitTest(float x, float y) noexcept override;

protected:
    void MeasureChildren(const LayoutSlot& available) noexcept override;
    void ArrangeChildren(const LayoutSlot& finalSlot) noexcept override;

private:
    float m_hScrollOffset{};
    float m_vScrollOffset{};
    float m_scrollableWidth{};
    float m_scrollableHeight{};
    bool m_hScrollEnabled{true};
    bool m_vScrollEnabled{true};

    void ClampScrollOffsets() noexcept;
    void RenderScrollBars(RenderContext& ctx) const noexcept;
};

} // namespace
