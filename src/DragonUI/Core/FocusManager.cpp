#include <DragonUI/Core/FocusManager.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Event.hpp>
#include <algorithm>

namespace DragonOS::DragonUI {

void FocusManager::SetFocus(Control* element) noexcept
{
    if (m_focused == element) return;

    if (m_focused)
    {
        FocusEventArgs args{};
        EventArgs evt;
        evt.type = EventType::LostFocus;
        evt.focus = {m_focused, element};
        (void)m_focused->OnEvent(evt);
        m_focused->SetControlState(ControlState::Normal);
    }

    m_focused = element;

    if (m_focused)
    {
        EventArgs evt;
        evt.type = EventType::GotFocus;
        evt.focus = {nullptr, m_focused};
        m_focused->SetControlState(ControlState::Focused);
        (void)m_focused->OnEvent(evt);
    }
}

bool FocusManager::IsFocused(const Element* element) const noexcept
{
    return m_focused == element;
}

void FocusManager::FocusNext() noexcept
{
    if (m_tabOrder.empty()) return;

    if (!m_focused)
    {
        FocusFirst();
        return;
    }

    auto it = std::find(m_tabOrder.begin(), m_tabOrder.end(), m_focused);
    if (it == m_tabOrder.end() || ++it == m_tabOrder.end())
        SetFocus(m_tabOrder.front());
    else
        SetFocus(*it);
}

void FocusManager::FocusPrevious() noexcept
{
    if (m_tabOrder.empty()) return;

    if (!m_focused)
    {
        FocusLast();
        return;
    }

    auto it = std::find(m_tabOrder.rbegin(), m_tabOrder.rend(), m_focused);
    if (it == m_tabOrder.rend() || ++it == m_tabOrder.rend())
        SetFocus(m_tabOrder.back());
    else
        SetFocus(*it);
}

void FocusManager::FocusFirst() noexcept
{
    if (!m_tabOrder.empty())
        SetFocus(m_tabOrder.front());
}

void FocusManager::FocusLast() noexcept
{
    if (!m_tabOrder.empty())
        SetFocus(m_tabOrder.back());
}

void FocusManager::RegisterRoot(Container* root) noexcept
{
    if (!root) return;
    auto it = std::find(m_roots.begin(), m_roots.end(), root);
    if (it == m_roots.end())
    {
        m_roots.push_back(root);
        RebuildTabOrder();
    }
}

void FocusManager::UnregisterRoot(Container* root) noexcept
{
    auto it = std::find(m_roots.begin(), m_roots.end(), root);
    if (it != m_roots.end())
    {
        m_roots.erase(it);
        if (m_focused && m_focused->GetParent() == root)
            m_focused = nullptr;
        RebuildTabOrder();
    }
}

void FocusManager::RebuildTabOrder() noexcept
{
    m_tabOrder.clear();

    for (auto* root : m_roots)
        CollectFocusable(root, m_tabOrder);

    std::stable_sort(m_tabOrder.begin(), m_tabOrder.end(),
        [](Control* a, Control* b) { return a->GetTabIndex() < b->GetTabIndex(); });
}

void FocusManager::CollectFocusable(Element* element, std::vector<Control*>& out) noexcept
{
    if (!element) return;

    if (auto* ctrl = dynamic_cast<Control*>(element))
    {
        if (ctrl->IsVisible() && ctrl->IsEnabled() && ctrl->IsFocusable())
            out.push_back(ctrl);
    }

    if (auto* container = dynamic_cast<Container*>(element))
    {
        for (const auto& child : container->GetChildren())
            CollectFocusable(child.get(), out);
    }
}

} // namespace
