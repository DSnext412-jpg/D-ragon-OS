#pragma once

#include <DragonUI/Accessibility/AccessibleElement.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::DragonUI {

class Element;

/**
 * @brief  The UI Automation tree for a DragonUI window.
 *
 * Mirrors the framework element tree with one automation node per element
 * and exposes:
 *   - tree traversal (root, parent, children, enumeration)
 *   - lookups by automation id, native id, element pointer or screen point
 *   - property inspection (role, state, value, name, description, bounds)
 *
 * The tree is rebuilt lazily via Rebuild() (typically after structural
 * changes, or before a screen-reader snapshot is taken).
 */
class UIAAutomationTree final {
public:
    UIAAutomationTree() noexcept = default;
    ~UIAAutomationTree() noexcept = default;

    UIAAutomationTree(const UIAAutomationTree&) = delete;
    UIAAutomationTree& operator=(const UIAAutomationTree&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────

    /// @brief  Set the root element and rebuild the tree immediately.
    void SetRoot(Element* root) noexcept;

    /// @brief  Rebuild the tree from the current root.
    void Rebuild() noexcept;

    /// @brief  Drop every node and the root reference.
    void Clear() noexcept;

    [[nodiscard]] Element* GetRootElement() const noexcept { return m_root; }

    // ── Access ─────────────────────────────────────────────────────────

    [[nodiscard]] AccessibleElement* GetRoot() const noexcept { return m_rootNode.get(); }

    [[nodiscard]] size_t GetNodeCount() const noexcept { return m_nodeCount; }
    [[nodiscard]] size_t GetDepth() const noexcept { return m_maxDepth; }

    /// @brief  Depth-first lookup by automation id (e.g. "0/2/5").
    [[nodiscard]] AccessibleElement* FindByAutomationId(std::wstring_view id) const noexcept;

    /// @brief  Lookup by the framework Element::GetId() value.
    [[nodiscard]] AccessibleElement* FindByNativeId(uint64_t nativeId) const noexcept;

    /// @brief  Lookup by the wrapped framework element pointer.
    [[nodiscard]] AccessibleElement* FindByElement(const Element* element) const noexcept;

    /// @brief  Hit-test: deepest node whose bounds contain @p (x, y).
    [[nodiscard]] AccessibleElement* FindByPoint(float x, float y) const noexcept;

    /// @brief  Depth-first enumeration.  @p visitor receives (node, depth).
    void Enumerate(std::function<void(AccessibleElement* node, size_t depth)> visitor) const noexcept;

    /// @brief  Collects every node that is currently visible in the tree.
    [[nodiscard]] std::vector<AccessibleElement*> CollectVisible() const noexcept;

private:
    friend class AccessibilityManager;

    std::unique_ptr<AccessibleElement> Build(Element* element, IAccessible* parent, const std::wstring& path) noexcept;
    void RebuildIndex() noexcept;
    void IndexNode(AccessibleElement* node) noexcept;

    Element* m_root{};
    std::unique_ptr<AccessibleElement> m_rootNode;
    std::unordered_map<std::wstring, AccessibleElement*> m_byAutomationId;
    std::unordered_map<uint64_t, AccessibleElement*> m_byNativeId;
    std::unordered_map<const Element*, AccessibleElement*> m_byElement;
    size_t m_nodeCount{};
    size_t m_maxDepth{};
};

} // namespace DragonOS::DragonUI