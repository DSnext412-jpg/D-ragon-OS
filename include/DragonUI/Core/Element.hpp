#pragma once

#include <DragonUI/Core/Layout.hpp>
#include <DragonUI/Core/Event.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <cstdint>
#include <string_view>

namespace DragonOS::DragonUI {

class Container;

class Element {
public:
    Element() noexcept;
    virtual ~Element() = default;

    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;
    Element(Element&&) = delete;
    Element& operator=(Element&&) = delete;

    [[nodiscard]] uint64_t GetId() const noexcept { return m_id; }

    // ── Layout ────────────────────────────────────────────────────────

    virtual void Measure(const LayoutSlot& available) noexcept;
    virtual void Arrange(const LayoutSlot& finalSlot) noexcept;
    virtual void Render(RenderContext& ctx) noexcept;

    [[nodiscard]] virtual Element* HitTest(float x, float y) noexcept;

    [[nodiscard]] virtual DesiredSize MeasureOverride(const LayoutSlot& available) noexcept;
    virtual void ArrangeOverride(const LayoutSlot& finalSlot) noexcept;

    void InvalidateLayout() noexcept { m_layoutDirty = true; }
    void InvalidateVisual() noexcept { m_visualDirty = true; }
    void Invalidate() noexcept { InvalidateLayout(); InvalidateVisual(); }

    [[nodiscard]] bool NeedsLayout() const noexcept { return m_layoutDirty; }
    [[nodiscard]] bool NeedsVisualUpdate() const noexcept { return m_visualDirty; }

    // ── Bounds ────────────────────────────────────────────────────────

    void SetBounds(const LayoutSlot& slot) noexcept { m_bounds = slot; Invalidate(); }
    [[nodiscard]] const LayoutSlot& GetBounds() const noexcept { return m_bounds; }
    [[nodiscard]] float GetX() const noexcept { return m_bounds.x; }
    [[nodiscard]] float GetY() const noexcept { return m_bounds.y; }
    [[nodiscard]] float GetWidth() const noexcept { return m_bounds.width; }
    [[nodiscard]] float GetHeight() const noexcept { return m_bounds.height; }
    [[nodiscard]] LayoutSlot GetContentSlot() const noexcept { return m_bounds.Inset(m_padding); }

    [[nodiscard]] DesiredSize GetDesiredSize() const noexcept { return m_desiredSize; }

    // ── Sizing ────────────────────────────────────────────────────────

    void SetMargin(const Thickness& m) noexcept { m_margin = m; InvalidateLayout(); }
    [[nodiscard]] const Thickness& GetMargin() const noexcept { return m_margin; }

    void SetPadding(const Thickness& p) noexcept { m_padding = p; InvalidateLayout(); }
    [[nodiscard]] const Thickness& GetPadding() const noexcept { return m_padding; }

    void SetMinSize(float w, float h) noexcept { m_minW = w; m_minH = h; InvalidateLayout(); }
    void SetMaxSize(float w, float h) noexcept { m_maxW = w; m_maxH = h; InvalidateLayout(); }
    [[nodiscard]] float GetMinWidth() const noexcept { return m_minW; }
    [[nodiscard]] float GetMaxWidth() const noexcept { return m_maxW; }
    [[nodiscard]] float GetMinHeight() const noexcept { return m_minH; }
    [[nodiscard]] float GetMaxHeight() const noexcept { return m_maxH; }

    // ── State ─────────────────────────────────────────────────────────

    void SetVisibility(Visibility v) noexcept { m_visibility = v; InvalidateLayout(); }
    [[nodiscard]] Visibility GetVisibility() const noexcept { return m_visibility; }
    [[nodiscard]] bool IsVisible() const noexcept { return m_visibility == Visibility::Visible; }

    void SetEnabled(bool enabled) noexcept { m_enabled = enabled; InvalidateVisual(); }
    [[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

    void SetOpacity(float opacity) noexcept { m_opacity = opacity; InvalidateVisual(); }
    [[nodiscard]] float GetOpacity() const noexcept { return m_opacity; }

    void SetZOrder(int z) noexcept { m_zOrder = z; InvalidateVisual(); }
    [[nodiscard]] int GetZOrder() const noexcept { return m_zOrder; }

    // ── Tree ──────────────────────────────────────────────────────────

    void SetParent(Container* parent) noexcept { m_parent = parent; }
    [[nodiscard]] Container* GetParent() const noexcept { return m_parent; }

    // ── Tag ───────────────────────────────────────────────────────────

    template<typename T>
    void SetUserData(T* data) noexcept { m_userData = data; }
    template<typename T>
    [[nodiscard]] T* GetUserData() const noexcept { return static_cast<T*>(m_userData); }

protected:
    uint64_t m_id;
    LayoutSlot m_bounds;
    DesiredSize m_desiredSize{};
    Thickness m_margin;
    Thickness m_padding;
    float m_minW{}, m_maxW{FLT_MAX};
    float m_minH{}, m_maxH{FLT_MAX};
    Visibility m_visibility{Visibility::Visible};
    bool m_enabled{true};
    float m_opacity{1.0f};
    int m_zOrder{};
    Container* m_parent{};
    bool m_layoutDirty{true};
    bool m_visualDirty{true};
    void* m_userData{};

private:
    static uint64_t s_nextId;
};

} // namespace
