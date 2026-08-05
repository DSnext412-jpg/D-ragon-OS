#pragma once

#include <DragonUI/Accessibility/IAccessible.hpp>

#include <memory>
#include <string>
#include <vector>

namespace DragonOS::DragonUI {

/**
 * @brief  Concrete accessible object that mirrors a DragonUI element.
 *
 * AccessibleElement is the base node of the accessibility tree.  Each node
 * wraps a framework Element* (or none, for virtual items such as list rows)
 * and exposes name, description, role, state, value, bounds and the
 * parent/child relationship.
 *
 * Subclass peers (see AutomationPeer.hpp) refine the role / state / value
 * for each control type.  Children are produced by BuildChildren(), which
 * follows the framework element tree through Container::GetChildren().
 */
class AccessibleElement : public IAccessible {
public:
    /// @brief  Wraps a real framework element.
    explicit AccessibleElement(Element* element) noexcept;

    /// @brief  Creates a virtual node (element == nullptr) for generated
    ///         content such as virtualized list rows.
    static std::unique_ptr<AccessibleElement> CreateVirtual() noexcept;

    ~AccessibleElement() override = default;

    // ── IAccessible ───────────────────────────────────────────────────

    [[nodiscard]] std::wstring_view GetAccessibleName() const noexcept override;
    [[nodiscard]] std::wstring_view GetAccessibleDescription() const noexcept override;
    [[nodiscard]] std::wstring_view GetAutomationId() const noexcept override;
    [[nodiscard]] uint64_t GetNativeId() const noexcept override;

    [[nodiscard]] AccessibilityRole GetAccessibleRole() const noexcept override;
    [[nodiscard]] AccessibilityState GetAccessibleState() const noexcept override;
    [[nodiscard]] std::wstring_view GetAccessibleValue() const noexcept override;
    [[nodiscard]] LayoutSlot GetAccessibleBounds() const noexcept override;

    [[nodiscard]] IAccessible* GetParent() const noexcept override { return m_parent; }
    [[nodiscard]] size_t GetChildCount() const noexcept override { return m_children.size(); }
    [[nodiscard]] IAccessible* GetChildAt(size_t index) const noexcept override;

    [[nodiscard]] Element* GetElement() const noexcept override { return m_element; }
    [[nodiscard]] Container* GetContainer() const noexcept override;

    // ── Construction / tree building ──────────────────────────────────

    /// @brief  Builds child nodes from the framework tree (or virtual items).
    /// @return true when at least one child was added.
    virtual bool BuildChildren() noexcept;

    /// @brief  Returns the child at @p index or nullptr when out of range.
    [[nodiscard]] AccessibleElement* ChildAt(size_t index) const noexcept;

    /// @brief  Appends a child node (ownership transfers).
    void AddChild(std::unique_ptr<IAccessible> child) noexcept { m_children.push_back(std::move(child)); }

    /// @brief  Sets the parent node (used while building virtual children).
    void SetParent(IAccessible* parent) noexcept { m_parent = parent; }

    // ── Node metadata (used by peers / virtual items) ─────────────────

    void SetRoleOverride(AccessibilityRole role) noexcept { m_roleOverride = role; }
    void SetNameOverride(std::wstring_view name) noexcept { m_nameOverride.assign(name); }
    void SetDescriptionOverride(std::wstring_view desc) noexcept { m_descriptionOverride.assign(desc); }
    void SetAutomationIdOverride(std::wstring_view id) noexcept { m_automationIdOverride.assign(id); }
    void SetValueOverride(std::wstring_view value) noexcept { m_valueOverride.assign(value); }
    void SetStateOverride(AccessibilityState state) noexcept { m_stateOverride = state; }

protected:
    friend class AutomationPeer;
    friend class UIAAutomationTree;

    // ── Peer extension points ─────────────────────────────────────────

    /// @brief  Resolves the control-specific role (default: type mapping).
    [[nodiscard]] virtual AccessibilityRole ResolveRole() const noexcept;

    /// @brief  Composes the control-specific state flags.
    [[nodiscard]] virtual AccessibilityState ComputeState() const noexcept;

    /// @brief  Provides the control-specific value text.
    [[nodiscard]] virtual std::wstring_view ComputeValue() const noexcept;

    /// @brief  Builds virtual child nodes (e.g. list rows).  Returns true
    ///         when virtual children were added.
    [[nodiscard]] virtual bool BuildVirtualChildren() noexcept { return false; }

    Element* m_element;
    IAccessible* m_parent{};

private:
    std::vector<std::unique_ptr<IAccessible>> m_children;

    // Optional per-node overrides (peers / virtual items).
    AccessibilityRole m_roleOverride{ AccessibilityRole::Unknown };
    AccessibilityState m_stateOverride{ AccessibilityState::None };
    std::wstring m_nameOverride;
    std::wstring m_descriptionOverride;
    std::wstring m_automationIdOverride;
    std::wstring m_valueOverride;
};

} // namespace DragonOS::DragonUI