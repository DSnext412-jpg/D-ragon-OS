#include <DragonUI/Accessibility/AccessibleElement.hpp>
#include <DragonUI/Accessibility/AutomationPeer.hpp>
#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/Element.hpp>

namespace DragonOS::DragonUI {

AccessibleElement::AccessibleElement(Element* element) noexcept
    : m_element(element)
{
}

std::unique_ptr<AccessibleElement> AccessibleElement::CreateVirtual() noexcept
{
    auto node = std::unique_ptr<AccessibleElement>(new AccessibleElement(nullptr));
    return node;
}

// ── IAccessible ───────────────────────────────────────────────────────────

std::wstring_view AccessibleElement::GetAccessibleName() const noexcept
{
    if (!m_nameOverride.empty())
        return m_nameOverride;
    if (m_element)
        return m_element->GetAccessibleName();
    return {};
}

std::wstring_view AccessibleElement::GetAccessibleDescription() const noexcept
{
    if (!m_descriptionOverride.empty())
        return m_descriptionOverride;
    if (m_element)
        return m_element->GetAccessibleDescription();
    return {};
}

std::wstring_view AccessibleElement::GetAutomationId() const noexcept
{
    if (!m_automationIdOverride.empty())
        return m_automationIdOverride;
    if (m_element)
        return m_element->GetAutomationId();
    return {};
}

uint64_t AccessibleElement::GetNativeId() const noexcept
{
    return m_element ? m_element->GetId() : 0;
}

AccessibilityRole AccessibleElement::GetAccessibleRole() const noexcept
{
    if (m_roleOverride != AccessibilityRole::Unknown)
        return m_roleOverride;
    return ResolveRole();
}

AccessibilityState AccessibleElement::GetAccessibleState() const noexcept
{
    AccessibilityState state = ComputeState();
    state |= m_stateOverride;

    if (m_element)
    {
        if (m_element->IsVisible())
            state |= AccessibilityState::Visible;
        if (!m_element->IsEnabled())
            state |= AccessibilityState::Disabled;
        else
            state |= AccessibilityState::Enabled;
        if (auto* ctrl = dynamic_cast<Control*>(m_element))
        {
            if (ctrl->IsFocusable())
                state |= AccessibilityState::Focusable;
        }
        if (m_parent == nullptr)
        {
            // Root nodes are always "visible" anchors; keep them Enabled.
            state |= AccessibilityState::Enabled;
        }
    }

    return state;
}

std::wstring_view AccessibleElement::GetAccessibleValue() const noexcept
{
    if (!m_valueOverride.empty())
        return m_valueOverride;
    return ComputeValue();
}

LayoutSlot AccessibleElement::GetAccessibleBounds() const noexcept
{
    if (m_element)
        return m_element->GetBounds();
    return {};
}

IAccessible* AccessibleElement::GetChildAt(size_t index) const noexcept
{
    if (index >= m_children.size())
        return nullptr;
    return m_children[index].get();
}

Container* AccessibleElement::GetContainer() const noexcept
{
    return dynamic_cast<Container*>(m_element);
}

// ── Construction / tree building ──────────────────────────────────────────

bool AccessibleElement::BuildChildren() noexcept
{
    if (BuildVirtualChildren())
        return true;

    if (!m_element)
        return false;

    auto* container = dynamic_cast<Container*>(m_element);
    if (!container)
        return false;

    const auto& children = container->GetChildren();
    for (const auto& child : children)
    {
        auto peer = AutomationPeerFactory(child.get());
        if (!peer)
            continue;

        peer->m_parent = this;
        peer->BuildChildren();
        m_children.push_back(std::move(peer));
    }
    return !m_children.empty();
}

AccessibleElement* AccessibleElement::ChildAt(size_t index) const noexcept
{
    if (index >= m_children.size())
        return nullptr;
    return static_cast<AccessibleElement*>(m_children[index].get());
}

// ── Peer extension points ─────────────────────────────────────────────────

AccessibilityRole AccessibleElement::ResolveRole() const noexcept
{
    if (!m_element)
        return AccessibilityRole::Unknown;
    return InferAccessibleRole(m_element);
}

AccessibilityState AccessibleElement::ComputeState() const noexcept
{
    return AccessibilityState::None;
}

std::wstring_view AccessibleElement::ComputeValue() const noexcept
{
    return {};
}

} // namespace DragonOS::DragonUI