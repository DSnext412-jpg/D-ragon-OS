#pragma once

#include <DragonUI/Core/Layout.hpp>
#include <DragonUI/Core/RenderContext.hpp>

#include <any>
#include <functional>

namespace DragonOS::DragonUI {

/**
 * @brief  Visual state passed to an item template at render time.
 */
struct ItemVisualState {
    bool selected{};
    bool hovered{};
    bool focused{};
    bool enabled{ true };
};

/**
 * @brief  Generic reusable item template.
 *
 * A template is a plain renderer plus the item value.  Keeping the
 * template decoupled from the data source makes it a natural seam for
 * future data binding: swap the source, keep the template.
 *
 * Usage:
 *   auto tpl = ItemTemplate<PluginInfo>([](RenderContext& ctx,
 *                                          const LayoutSlot& slot,
 *                                          const PluginInfo& p,
 *                                          const ItemVisualState& vs) {
 *       // custom draw for p
 *   });
 */
template<typename T>
class ItemTemplate final {
public:
    using RenderFn = std::function<void(RenderContext&, const LayoutSlot&, const T&, const ItemVisualState&)>;

    ItemTemplate() noexcept = default;
    explicit ItemTemplate(RenderFn fn) noexcept
        : m_render(std::move(fn)) {}

    void Render(RenderContext& ctx, const LayoutSlot& slot, const T& item, const ItemVisualState& state) const {
        if (m_render)
            m_render(ctx, slot, item, state);
    }

    [[nodiscard]] bool IsValid() const noexcept { return static_cast<bool>(m_render); }
    explicit operator bool() const noexcept { return m_render.operator bool(); }

private:
    RenderFn m_render;
};

/**
 * @brief  Type-erased item template used by controls internally.
 *
 * The controls store an erased renderer so they never depend on a
 * concrete data type, yet still accept strongly-typed templates from
 * callers via @ref MakeErasedItemTemplate.
 */
using ErasedItemRenderer = std::function<void(RenderContext&, const LayoutSlot&, const std::any&, const ItemVisualState&)>;

template<typename T>
[[nodiscard]] inline ErasedItemRenderer MakeErasedItemRenderer(const ItemTemplate<T>& tpl) noexcept
{
    if (!tpl)
        return {};

    return [tpl](RenderContext& ctx, const LayoutSlot& slot, const std::any& value, const ItemVisualState& state) {
        if (const auto* item = std::any_cast<T>(&value))
            tpl.Render(ctx, slot, *item, state);
    };
}

} // namespace DragonOS::DragonUI
