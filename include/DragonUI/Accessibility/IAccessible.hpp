#pragma once

#include <DragonUI/Accessibility/AccessibilityRole.hpp>
#include <DragonUI/Accessibility/AccessibilityState.hpp>
#include <DragonUI/Core/Layout.hpp>

#include <cstdint>
#include <memory>
#include <string_view>

namespace DragonOS::DragonUI {

class Element;
class Container;

/**
 * @brief  Pure interface for an accessible / automatable UI element.
 *
 * Every accessible object in DragonUI (real controls, virtual list rows,
 * dialog surfaces, and the window itself) implements this interface.  A
 * UI Automation provider can walk the tree through GetParent / GetChildAt
 * and inspect each node through the property accessors.
 */
class IAccessible {
public:
    virtual ~IAccessible() = default;

    // ── Identity ──────────────────────────────────────────────────────

    /// @brief  Human-readable accessible name (e.g. "Save", "Username").
    [[nodiscard]] virtual std::wstring_view GetAccessibleName() const noexcept = 0;

    /// @brief  Supplementary description / help text.
    [[nodiscard]] virtual std::wstring_view GetAccessibleDescription() const noexcept = 0;

    /// @brief  Stable automation identifier (may be empty).
    [[nodiscard]] virtual std::wstring_view GetAutomationId() const noexcept = 0;

    /// @brief  Framework-native element id (see Element::GetId).
    [[nodiscard]] virtual uint64_t GetNativeId() const noexcept = 0;

    // ── Semantics ─────────────────────────────────────────────────────

    [[nodiscard]] virtual AccessibilityRole GetAccessibleRole() const noexcept = 0;
    [[nodiscard]] virtual AccessibilityState GetAccessibleState() const noexcept = 0;

    /// @brief  Current value; empty when the element has no value.
    [[nodiscard]] virtual std::wstring_view GetAccessibleValue() const noexcept = 0;

    /// @brief  Bounding rectangle in element coordinates (DIPs).
    [[nodiscard]] virtual LayoutSlot GetAccessibleBounds() const noexcept = 0;

    // ── Tree ──────────────────────────────────────────────────────────

    [[nodiscard]] virtual IAccessible* GetParent() const noexcept = 0;
    [[nodiscard]] virtual size_t GetChildCount() const noexcept = 0;
    [[nodiscard]] virtual IAccessible* GetChildAt(size_t index) const noexcept = 0;

    // ── Native access ─────────────────────────────────────────────────

    /// @brief  The underlying framework element (null for virtual nodes).
    [[nodiscard]] virtual Element* GetElement() const noexcept = 0;

    /// @brief  Traverses up to the nearest real framework container.
    [[nodiscard]] virtual Container* GetContainer() const noexcept = 0;
};

} // namespace DragonOS::DragonUI