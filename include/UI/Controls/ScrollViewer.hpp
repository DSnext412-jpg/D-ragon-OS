#pragma once
#include <UI/Core/UIContainer.hpp>

namespace DragonOS::UI {

class ScrollViewer : public UIContainer {
public:
    ScrollViewer() noexcept;

    void SetContent(std::unique_ptr<UIElement> content) noexcept;
    [[nodiscard]] UIElement* GetContent() const noexcept;

    void SetScrollX(float x) noexcept;
    [[nodiscard]] float GetScrollX() const noexcept { return m_scrollX; }
    void SetScrollY(float y) noexcept;
    [[nodiscard]] float GetScrollY() const noexcept { return m_scrollY; }

    void SetScrollBarWidth(float width) noexcept { m_scrollBarWidth = width; InvalidateLayout(); }
    [[nodiscard]] float GetScrollBarWidth() const noexcept { return m_scrollBarWidth; }

    void Measure(const D2D1_RECT_F& availableSize) noexcept override;
    void Arrange(const D2D1_RECT_F& finalRect) noexcept override;
    void Render(UIRenderer& renderer) noexcept override;
    bool OnEvent(const UIEvent& event) noexcept override;

    static constexpr float DefaultScrollBarWidth = 12.0f;
    static constexpr float MinScrollBarHeight = 20.0f;

private:
    void RenderScrollBar(UIRenderer& renderer, const D2D1_RECT_F& barRect, float contentExtent, float viewportExtent, float scrollPos, bool vertical) noexcept;

    float m_scrollX{0};
    float m_scrollY{0};
    float m_scrollBarWidth{DefaultScrollBarWidth};
    D2D1_SIZE_F m_contentSize{};
    bool m_draggingHScroll{false};
    bool m_draggingVScroll{false};
    float m_dragStartPos{0};
    float m_dragStartScroll{0};
};

} // namespace
