#include <DragonUI/Accessibility/UIAAutomationTree.hpp>
#include <DragonUI/Accessibility/AutomationPeer.hpp>
#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Element.hpp>

#include <algorithm>

namespace DragonOS::DragonUI {

void UIAAutomationTree::SetRoot(Element* root) noexcept
{
    m_root = root;
    Rebuild();
}

void UIAAutomationTree::Rebuild() noexcept
{
    m_rootNode.reset();
    m_nodeCount = 0;
    m_maxDepth = 0;

    if (!m_root)
        return;

    m_rootNode = Build(m_root, nullptr, L"0");
    if (m_rootNode)
    {
        m_rootNode->SetRoleOverride(AccessibilityRole::Window);
        m_rootNode->BuildChildren();
        RebuildIndex();
    }
}

void UIAAutomationTree::Clear() noexcept
{
    m_rootNode.reset();
    m_root = nullptr;
    m_byAutomationId.clear();
    m_byNativeId.clear();
    m_byElement.clear();
    m_nodeCount = 0;
    m_maxDepth = 0;
}

// ── Node building ─────────────────────────────────────────────────────────

std::unique_ptr<AccessibleElement> UIAAutomationTree::Build(
    Element* element, IAccessible* parent, const std::wstring& path) noexcept
{
    if (!element)
        return nullptr;

    auto node = AutomationPeerFactory(element);
    if (!node)
        return nullptr;

    node->m_parent = parent;
    node->SetAutomationIdOverride(path);
    ++m_nodeCount;
    return node;
}

void UIAAutomationTree::RebuildIndex() noexcept
{
    m_byAutomationId.clear();
    m_byNativeId.clear();
    m_byElement.clear();
    m_maxDepth = 0;

    if (!m_rootNode)
        return;

    IndexNode(m_rootNode.get());
}

void UIAAutomationTree::IndexNode(AccessibleElement* node) noexcept
{
    if (!node)
        return;

    size_t depth = 0;
    for (IAccessible* it = node->GetParent(); it; it = it->GetParent())
        ++depth;
    m_maxDepth = (std::max)(m_maxDepth, depth + 1);

    const std::wstring_view automationId = node->GetAutomationId();
    if (!automationId.empty())
        m_byAutomationId.emplace(automationId, node);

    if (node->GetNativeId() != 0)
        m_byNativeId.emplace(node->GetNativeId(), node);

    if (node->GetElement())
        m_byElement.emplace(node->GetElement(), node);

    for (size_t i = 0; i < node->GetChildCount(); ++i)
    {
        auto* child = static_cast<AccessibleElement*>(node->GetChildAt(i));
        if (child)
            IndexNode(child);
    }
}

// ── Lookups ───────────────────────────────────────────────────────────────

AccessibleElement* UIAAutomationTree::FindByAutomationId(std::wstring_view id) const noexcept
{
    if (id.empty())
        return nullptr;
    auto it = m_byAutomationId.find(std::wstring(id));
    return it != m_byAutomationId.end() ? it->second : nullptr;
}

AccessibleElement* UIAAutomationTree::FindByNativeId(uint64_t nativeId) const noexcept
{
    auto it = m_byNativeId.find(nativeId);
    return it != m_byNativeId.end() ? it->second : nullptr;
}

AccessibleElement* UIAAutomationTree::FindByElement(const Element* element) const noexcept
{
    if (!element)
        return nullptr;
    auto it = m_byElement.find(element);
    return it != m_byElement.end() ? it->second : nullptr;
}

AccessibleElement* UIAAutomationTree::FindByPoint(float x, float y) const noexcept
{
    if (!m_rootNode)
        return nullptr;

    AccessibleElement* best = nullptr;
    size_t bestDepth = 0;

    const auto visit = [&](AccessibleElement* node, size_t depth) noexcept {
        if (!node)
            return;
        const LayoutSlot bounds = node->GetAccessibleBounds();
        if (bounds.Contains(x, y) && depth >= bestDepth)
        {
            best = node;
            bestDepth = depth;
        }
    };

    Enumerate([&](AccessibleElement* node, size_t depth) { visit(node, depth); });
    return best;
}

// ── Enumeration ───────────────────────────────────────────────────────────

void UIAAutomationTree::Enumerate(
    std::function<void(AccessibleElement*, size_t)> visitor) const noexcept
{
    const auto walk = [&](AccessibleElement* node, size_t depth, auto& self) -> void {
        if (!node)
            return;
        visitor(node, depth);
        for (size_t i = 0; i < node->GetChildCount(); ++i)
        {
            auto* child = static_cast<AccessibleElement*>(node->GetChildAt(i));
            if (child)
                self(child, depth + 1, self);
        }
    };

    if (m_rootNode)
        walk(m_rootNode.get(), 0, walk);
}

std::vector<AccessibleElement*> UIAAutomationTree::CollectVisible() const noexcept
{
    std::vector<AccessibleElement*> result;
    const auto visit = [&](AccessibleElement* node, size_t) noexcept {
        if (!node)
            return;
        const Element* element = node->GetElement();
        if (!element || element->IsVisible())
            result.push_back(node);
    };
    Enumerate(visit);
    return result;
}

} // namespace DragonOS::DragonUI