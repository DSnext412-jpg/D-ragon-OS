#include <DragonUI/Accessibility/AccessibilityNotification.hpp>
#include <DragonUI/Accessibility/AccessibilityPreferences.hpp>

#include <algorithm>

namespace DragonOS::DragonUI {

// ── AccessibilityNotificationHub ──────────────────────────────────────────

AccessibilityNotificationHub::Subscription AccessibilityNotificationHub::Subscribe(
    Listener listener) noexcept
{
    const auto id = m_nextId++;
    m_listeners.push_back(Entry{ id, std::move(listener) });
    return Subscription{ this, id };
}

void AccessibilityNotificationHub::Publish(const AccessibilityEventArgs& args) noexcept
{
    // Copy callbacks so listeners may unsubscribe during iteration.
    std::vector<Listener> callbacks;
    callbacks.reserve(m_listeners.size());
    for (const auto& entry : m_listeners)
        callbacks.push_back(entry.listener);

    for (const auto& cb : callbacks)
    {
        if (cb)
            cb(args);
    }
}

void AccessibilityNotificationHub::Notify(AccessibilityEvent type, Element* element,
                                          std::wstring_view name,
                                          std::wstring_view value) noexcept
{
    AccessibilityEventArgs args;
    args.type = type;
    args.element = element;
    args.name = name;
    args.value = value;
    Publish(args);
}

void AccessibilityNotificationHub::Remove(uint64_t id) noexcept
{
    for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
    {
        if (it->id == id)
        {
            m_listeners.erase(it);
            return;
        }
    }
}

// ── AccessibilityPreferences ──────────────────────────────────────────────

void AccessibilityPreferences::SetHighContrast(bool enabled) noexcept
{
    if (m_highContrast == enabled)
        return;
    m_highContrast = enabled;
    Notify();
}

void AccessibilityPreferences::SetReducedMotion(bool enabled) noexcept
{
    if (m_reducedMotion == enabled)
        return;
    m_reducedMotion = enabled;
    Notify();
}

void AccessibilityPreferences::SetTextScale(float scale) noexcept
{
    const float clamped = (std::max)(1.0f, (std::min)(2.0f, scale));
    if (m_textScale == clamped)
        return;
    m_textScale = clamped;
    Notify();
}

void AccessibilityPreferences::SetScreenReaderActive(bool active) noexcept
{
    if (m_screenReader == active)
        return;
    m_screenReader = active;
    Notify();
}

AccessibilityPreferences::ChangeListener AccessibilityPreferences::AddListener(
    ChangedCallback cb) noexcept
{
    const auto id = m_nextId++;
    m_listeners.push_back(Entry{ id, std::move(cb) });
    return ChangeListener{ this, id };
}

void AccessibilityPreferences::RemoveListener(uint64_t id) noexcept
{
    for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
    {
        if (it->id == id)
        {
            m_listeners.erase(it);
            return;
        }
    }
}

void AccessibilityPreferences::Notify() noexcept
{
    std::vector<ChangedCallback> callbacks;
    callbacks.reserve(m_listeners.size());
    for (const auto& entry : m_listeners)
        callbacks.push_back(entry.cb);
    for (const auto& cb : callbacks)
    {
        if (cb)
            cb(*this);
    }
}

} // namespace DragonOS::DragonUI