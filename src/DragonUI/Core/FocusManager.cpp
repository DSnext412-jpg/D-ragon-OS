#include <DragonUI/Core/FocusManager.hpp>
#include <DragonUI/Core/Control.hpp>
#include <DragonUI/Core/Container.hpp>
#include <DragonUI/Core/Event.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

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

    if (m_onFocusChanged)
        m_onFocusChanged(m_focused);
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

// ── Logical arrow-key navigation ──────────────────────────────────────────

void FocusManager::MoveFocusDirection(FocusDirection direction) noexcept
{
    if (m_tabOrder.empty())
        return;

    Control* current = m_focused;
    if (!current)
    {
        FocusFirst();
        return;
    }

    const auto bounds = current->GetBounds();
    const float curCx = bounds.x + bounds.width * 0.5f;
    const float curCy = bounds.y + bounds.height * 0.5f;

    Control* best = nullptr;
    float bestScore = std::numeric_limits<float>::max();

    for (Control* candidate : m_tabOrder)
    {
        if (candidate == current)
            continue;

        const auto b = candidate->GetBounds();
        const float cx = b.x + b.width * 0.5f;
        const float cy = b.y + b.height * 0.5f;
        const float dx = cx - curCx;
        const float dy = cy - curCy;

        // Perpendicular overlap window (generous for small controls).
        const float perpWindow = 48.0f;
        float ahead{};
        float perp{};

        switch (direction)
        {
        case FocusDirection::Right:
            ahead = dx;
            perp = std::fabs(dy);
            break;
        case FocusDirection::Left:
            ahead = -dx;
            perp = std::fabs(dy);
            break;
        case FocusDirection::Down:
            ahead = dy;
            perp = std::fabs(dx);
            break;
        case FocusDirection::Up:
            ahead = -dy;
            perp = std::fabs(dx);
            break;
        }

        if (ahead <= 0.0f)
            continue;

        const float score = ahead + perp * 2.0f +
            (perp > perpWindow ? 10000.0f : 0.0f);
        if (score < bestScore)
        {
            bestScore = score;
            best = candidate;
        }
    }

    if (best)
        SetFocus(best);
}

Control* FocusManager::FindByAccessKey(wchar_t key, const Control* skip) const noexcept
{
    if (key == 0)
        return nullptr;

    wchar_t lower = key;
    if (lower >= L'A' && lower <= L'Z')
        lower = static_cast<wchar_t>(lower - L'A' + L'a');

    for (Control* candidate : m_tabOrder)
    {
        if (candidate == skip || !candidate->IsVisible() || !candidate->IsEnabled())
            continue;
        const wchar_t accessKey = candidate->GetAccessKey();
        if (accessKey == 0)
            continue;
        wchar_t k = accessKey;
        if (k >= L'A' && k <= L'Z')
            k = static_cast<wchar_t>(k - L'A' + L'a');
        if (k == lower)
            return candidate;
    }
    return nullptr;
}

void FocusManager::ActivateFocused() noexcept
{
    if (!m_focused)
        return;

    // Synthesise a click so invokable controls (buttons, menu items) react.
    EventArgs args = EventArgs::MakeMouse(
        EventType::Click,
        m_focused->GetX() + m_focused->GetWidth() * 0.5f,
        m_focused->GetY() + m_focused->GetHeight() * 0.5f,
        Input::MouseButton::Left, 1);
    (void)m_focused->OnEvent(args);
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
