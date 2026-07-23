#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <limits>
#include <d2d1.h>
#include "UIStyle.hpp"
#include "UIEvent.hpp"

namespace DragonOS::UI {

class UIContainer;
class UIRenderer;

enum class Visibility : uint8_t { Visible, Hidden, Collapsed };
enum class ElementState : uint8_t { Normal, Hover, Pressed, Focused, Disabled };

class UIElement {
public:
    UIElement() noexcept;
    virtual ~UIElement() = default;

    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;
    UIElement(UIElement&&) = delete;
    UIElement& operator=(UIElement&&) = delete;

    [[nodiscard]] uint64_t GetId() const noexcept { return m_id; }

    void SetBounds(const D2D1_RECT_F& bounds) noexcept;
    [[nodiscard]] const D2D1_RECT_F& GetBounds() const noexcept { return m_bounds; }
    [[nodiscard]] float GetX() const noexcept { return m_bounds.left; }
    [[nodiscard]] float GetY() const noexcept { return m_bounds.top; }
    [[nodiscard]] float GetWidth() const noexcept { return m_bounds.right - m_bounds.left; }
    [[nodiscard]] float GetHeight() const noexcept { return m_bounds.bottom - m_bounds.top; }
    void SetPosition(float x, float y) noexcept;
    void SetSize(float width, float height) noexcept;

    void SetDesiredSize(const D2D1_RECT_F& size) noexcept { m_desiredSize = size; }
    [[nodiscard]] const D2D1_RECT_F& GetDesiredSize() const noexcept { return m_desiredSize; }

    void SetMargin(float left, float top, float right, float bottom) noexcept;
    void SetMargin(float uniform) noexcept { SetMargin(uniform, uniform, uniform, uniform); }
    [[nodiscard]] float GetMarginLeft() const noexcept { return m_margin[0]; }
    [[nodiscard]] float GetMarginTop() const noexcept { return m_margin[1]; }
    [[nodiscard]] float GetMarginRight() const noexcept { return m_margin[2]; }
    [[nodiscard]] float GetMarginBottom() const noexcept { return m_margin[3]; }

    void SetPadding(float left, float top, float right, float bottom) noexcept;
    void SetPadding(float uniform) noexcept { SetPadding(uniform, uniform, uniform, uniform); }
    [[nodiscard]] float GetPaddingLeft() const noexcept { return m_padding[0]; }
    [[nodiscard]] float GetPaddingTop() const noexcept { return m_padding[1]; }
    [[nodiscard]] float GetPaddingRight() const noexcept { return m_padding[2]; }
    [[nodiscard]] float GetPaddingBottom() const noexcept { return m_padding[3]; }
    [[nodiscard]] D2D1_RECT_F GetPaddingRect() const noexcept;

    void SetMinSize(float w, float h) noexcept { m_minWidth = w; m_minHeight = h; }
    void SetMaxSize(float w, float h) noexcept { m_maxWidth = w; m_maxHeight = h; }
    [[nodiscard]] float GetMinWidth() const noexcept { return m_minWidth; }
    [[nodiscard]] float GetMaxWidth() const noexcept { return m_maxWidth; }
    [[nodiscard]] float GetMinHeight() const noexcept { return m_minHeight; }
    [[nodiscard]] float GetMaxHeight() const noexcept { return m_maxHeight; }

    void SetVisibility(Visibility v) noexcept { m_visibility = v; InvalidateLayout(); }
    [[nodiscard]] Visibility GetVisibility() const noexcept { return m_visibility; }
    [[nodiscard]] bool IsVisible() const noexcept { return m_visibility == Visibility::Visible; }

    void SetEnabled(bool enabled) noexcept;
    [[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

    void SetOpacity(float opacity) noexcept { m_opacity = opacity; InvalidateVisual(); }
    [[nodiscard]] float GetOpacity() const noexcept { return m_opacity; }

    void SetZIndex(int z) noexcept { m_zIndex = z; }
    [[nodiscard]] int GetZIndex() const noexcept { return m_zIndex; }

    [[nodiscard]] ElementState GetState() const noexcept { return m_state; }
    void SetState(ElementState state) noexcept;

    void SetFocusable(bool focusable) noexcept { m_focusable = focusable; }
    [[nodiscard]] bool IsFocusable() const noexcept { return m_focusable; }
    void SetTabIndex(int index) noexcept { m_tabIndex = index; }
    [[nodiscard]] int GetTabIndex() const noexcept { return m_tabIndex; }

    void SetParent(UIContainer* parent) noexcept { m_parent = parent; }
    [[nodiscard]] UIContainer* GetParent() const noexcept { return m_parent; }

    void SetStyle(std::shared_ptr<UIStyle> style) noexcept { m_style = std::move(style); InvalidateVisual(); }
    [[nodiscard]] std::shared_ptr<UIStyle> GetStyle() const noexcept { return m_style; }

    void SetTooltip(std::wstring_view tooltip) noexcept { m_tooltip = tooltip; }
    [[nodiscard]] const std::wstring& GetTooltip() const noexcept { return m_tooltip; }

    void SetAccessibilityRole(std::wstring_view role) noexcept { m_accessibilityRole = role; }
    [[nodiscard]] const std::wstring& GetAccessibilityRole() const noexcept { return m_accessibilityRole; }
    void SetAccessibilityLabel(std::wstring_view label) noexcept { m_accessibilityLabel = label; }
    [[nodiscard]] const std::wstring& GetAccessibilityLabel() const noexcept { return m_accessibilityLabel; }

    virtual void Measure(const D2D1_RECT_F& availableSize) noexcept;
    virtual void Arrange(const D2D1_RECT_F& finalRect) noexcept;
    virtual void Render(UIRenderer& renderer) noexcept;
    [[nodiscard]] virtual UIElement* HitTest(float x, float y) noexcept;
    [[nodiscard]] virtual bool OnEvent(const UIEvent& event) noexcept;

    [[nodiscard]] virtual D2D1_RECT_F MeasureOverride(const D2D1_RECT_F& availableSize) noexcept;
    virtual void ArrangeOverride(const D2D1_RECT_F& finalRect) noexcept;

    void SetRenderTransform(float offsetX, float offsetY, float scaleX, float scaleY) noexcept;
    [[nodiscard]] float GetRenderOffsetX() const noexcept { return m_renderOffsetX; }
    [[nodiscard]] float GetRenderOffsetY() const noexcept { return m_renderOffsetY; }
    [[nodiscard]] float GetRenderScaleX() const noexcept { return m_renderScaleX; }
    [[nodiscard]] float GetRenderScaleY() const noexcept { return m_renderScaleY; }

    void SetAnimatedOpacity(float opacity) noexcept { m_animOpacity = opacity; InvalidateVisual(); }
    [[nodiscard]] float GetAnimatedOpacity() const noexcept { return m_animOpacity; }
    void SetAnimatedOffset(float x, float y) noexcept { m_animOffsetX = x; m_animOffsetY = y; InvalidateVisual(); }
    [[nodiscard]] float GetAnimatedOffsetX() const noexcept { return m_animOffsetX; }
    [[nodiscard]] float GetAnimatedOffsetY() const noexcept { return m_animOffsetY; }
    void SetAnimatedScale(float s) noexcept { m_animScale = s; InvalidateVisual(); }
    [[nodiscard]] float GetAnimatedScale() const noexcept { return m_animScale; }

    void SetTag(void* tag) noexcept { m_tag = tag; }
    [[nodiscard]] void* GetTag() const noexcept { return m_tag; }

protected:
    void InvalidateLayout() noexcept { m_layoutDirty = true; }
    void InvalidateVisual() noexcept { m_visualDirty = true; }
    [[nodiscard]] bool NeedsLayout() const noexcept { return m_layoutDirty; }
    [[nodiscard]] bool NeedsVisualUpdate() const noexcept { return m_visualDirty; }

    uint64_t m_id;
    D2D1_RECT_F m_bounds{};
    D2D1_RECT_F m_desiredSize{};
    float m_margin[4]{};
    float m_padding[4]{};
    float m_minWidth{0}, m_maxWidth{FLT_MAX};
    float m_minHeight{0}, m_maxHeight{FLT_MAX};
    Visibility m_visibility{Visibility::Visible};
    ElementState m_state{ElementState::Normal};
    bool m_enabled{true};
    float m_opacity{1.0f};
    int m_zIndex{0};
    bool m_focusable{false};
    int m_tabIndex{0};
    UIContainer* m_parent{nullptr};
    std::shared_ptr<UIStyle> m_style;
    std::wstring m_tooltip;
    std::wstring m_accessibilityRole;
    std::wstring m_accessibilityLabel;
    bool m_layoutDirty{true};
    bool m_visualDirty{true};
    float m_renderOffsetX{0}, m_renderOffsetY{0};
    float m_renderScaleX{1}, m_renderScaleY{1};
    float m_animOpacity{1.0f};
    float m_animOffsetX{0}, m_animOffsetY{0};
    float m_animScale{1.0f};
    void* m_tag{nullptr};
    static uint64_t s_nextId;
};

} // namespace
