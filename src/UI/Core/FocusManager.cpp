#include <UI/Core/FocusManager.hpp>
#include <UI/Core/UIElement.hpp>
#include <UI/Core/UIContainer.hpp>
#include <UI/Core/UIEvent.hpp>
#include <algorithm>

namespace DragonOS::UI {

void FocusManager::SetFocusedElement(UIElement* element) noexcept
{
    if (m_focused == element)
        return;

    if (m_focused)
    {
        UIEvent lost{};
        lost.type   = UIEventType::FocusLost;
        lost.target = m_focused;
        (void)m_focused->OnEvent(lost);
    }

    m_focused = element;

    if (m_focused)
    {
        UIEvent gained{};
        gained.type   = UIEventType::FocusGained;
        gained.target = m_focused;
        (void)m_focused->OnEvent(gained);
    }
}

void FocusManager::FocusNext() noexcept
{
    if (m_tabOrder.empty())
        return;

    if (!m_focused)
    {
        FocusFirst();
        return;
    }

    auto it = std::find(m_tabOrder.begin(), m_tabOrder.end(), m_focused);
    if (it == m_tabOrder.end() || ++it == m_tabOrder.end())
    {
        SetFocusedElement(m_tabOrder.front());
    }
    else
    {
        SetFocusedElement(*it);
    }
}

void FocusManager::FocusPrevious() noexcept
{
    if (m_tabOrder.empty())
        return;

    if (!m_focused)
    {
        FocusLast();
        return;
    }

    auto it = std::find(m_tabOrder.rbegin(), m_tabOrder.rend(), m_focused);
    if (it == m_tabOrder.rend() || ++it == m_tabOrder.rend())
    {
        SetFocusedElement(m_tabOrder.back());
    }
    else
    {
        SetFocusedElement(*it);
    }
}

void FocusManager::FocusFirst() noexcept
{
    if (!m_tabOrder.empty())
    {
        SetFocusedElement(m_tabOrder.front());
    }
}

void FocusManager::FocusLast() noexcept
{
    if (!m_tabOrder.empty())
    {
        SetFocusedElement(m_tabOrder.back());
    }
}

void FocusManager::RegisterRoot(UIContainer* root) noexcept
{
    if (!root)
        return;

    auto it = std::find(m_roots.begin(), m_roots.end(), root);
    if (it == m_roots.end())
    {
        m_roots.push_back(root);
        UpdateTabOrder();
    }
}

void FocusManager::UnregisterRoot(UIContainer* root) noexcept
{
    auto it = std::find(m_roots.begin(), m_roots.end(), root);
    if (it != m_roots.end())
    {
        m_roots.erase(it);
        if (m_focused && m_focused->GetParent() == root)
        {
            m_focused = nullptr;
        }
        UpdateTabOrder();
    }
}

void FocusManager::UpdateTabOrder() noexcept
{
    m_tabOrder.clear();

    for (auto* root : m_roots)
    {
        CollectFocusable(root, m_tabOrder);
    }

    std::stable_sort(m_tabOrder.begin(), m_tabOrder.end(),
        [](UIElement* a, UIElement* b)
        {
            return a->GetTabIndex() < b->GetTabIndex();
        });
}

void FocusManager::CollectFocusable(
    UIElement* element,
    std::vector<UIElement*>& out) noexcept
{
    if (!element)
        return;

    if (element->IsVisible() && element->IsEnabled() && element->IsFocusable())
    {
        out.push_back(element);
    }

    if (auto* container = dynamic_cast<UIContainer*>(element))
    {
        for (const auto& child : container->GetChildren())
        {
            CollectFocusable(child.get(), out);
        }
    }
}

} // namespace
