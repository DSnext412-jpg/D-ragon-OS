#include <Notifications/NotificationManager.hpp>

namespace DragonOS::Notifications {

bool NotificationManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_notifications.reserve(MaxHistory);
    m_initialized = true;
    return true;
}

void NotificationManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }
    m_notifications.clear();
    m_activeCount = 0;
    m_unreadCount = 0;
    m_initialized = false;
}

void NotificationManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    const auto now = std::chrono::steady_clock::now();
    m_activeCount = 0;
    m_unreadCount = 0;

    for (auto& n : m_notifications)
    {
        if (n.dismissed) { continue; }

        ++m_activeCount;
        if (!n.read) { ++m_unreadCount; }

        if (n.dismissMode == NotificationDismiss::Automatic)
        {
            const auto elapsed = std::chrono::duration<float>(now - n.timestamp).count();
            if (elapsed >= AutoDismissDuration)
            {
                n.dismissed = true;
                --m_activeCount;
                if (m_onDismiss) { m_onDismiss(n); }
            }
        }
    }

    TrimHistory();
}

void NotificationManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void NotificationManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

uint64_t NotificationManager::Show(Notification notification) noexcept
{
    if (!m_initialized) { return 0; }

    notification.id = NextId();
    notification.timestamp = std::chrono::steady_clock::now();

    m_notifications.push_back(std::move(notification));
    ++m_activeCount;
    ++m_unreadCount;

    if (m_onShow)
    {
        m_onShow(m_notifications.back());
    }

    return m_notifications.back().id;
}

bool NotificationManager::Dismiss(uint64_t id) noexcept
{
    for (auto& n : m_notifications)
    {
        if (n.id == id && !n.dismissed)
        {
            n.dismissed = true;
            if (m_activeCount > 0) { --m_activeCount; }
            if (m_onDismiss) { m_onDismiss(n); }
            return true;
        }
    }
    return false;
}

bool NotificationManager::DismissGroup(std::wstring_view groupKey) noexcept
{
    bool any = false;
    for (auto& n : m_notifications)
    {
        if (n.groupKey == groupKey && !n.dismissed)
        {
            n.dismissed = true;
            if (m_activeCount > 0) { --m_activeCount; }
            any = true;
        }
    }
    return any;
}

void NotificationManager::DismissAll() noexcept
{
    for (auto& n : m_notifications)
    {
        if (!n.dismissed)
        {
            n.dismissed = true;
        }
    }
    m_activeCount = 0;
}

void NotificationManager::MarkRead(uint64_t id) noexcept
{
    for (auto& n : m_notifications)
    {
        if (n.id == id && !n.read)
        {
            n.read = true;
            if (m_unreadCount > 0) { --m_unreadCount; }
            break;
        }
    }
}

const Notification* NotificationManager::Find(uint64_t id) const noexcept
{
    for (const auto& n : m_notifications)
    {
        if (n.id == id) { return &n; }
    }
    return nullptr;
}

std::vector<const Notification*> NotificationManager::GetActive() const noexcept
{
    std::vector<const Notification*> result;
    for (const auto& n : m_notifications)
    {
        if (!n.dismissed) { result.push_back(&n); }
    }
    return result;
}

std::vector<const Notification*> NotificationManager::GetHistory() const noexcept
{
    std::vector<const Notification*> result;
    for (const auto& n : m_notifications)
    {
        if (n.dismissed) { result.push_back(&n); }
    }
    return result;
}

std::vector<const Notification*> NotificationManager::GetGrouped(std::wstring_view groupKey) const noexcept
{
    std::vector<const Notification*> result;
    for (const auto& n : m_notifications)
    {
        if (n.groupKey == groupKey) { result.push_back(&n); }
    }
    return result;
}

void NotificationManager::TrimHistory() noexcept
{
    if (m_notifications.size() <= MaxHistory) { return; }

    size_t dismissedCount = 0;
    for (const auto& n : m_notifications)
    {
        if (n.dismissed) { ++dismissedCount; }
    }

    const size_t toRemove = m_notifications.size() - MaxHistory;
    size_t removed = 0;

    for (auto it = m_notifications.begin(); it != m_notifications.end() && removed < toRemove; )
    {
        if (it->dismissed)
        {
            it = m_notifications.erase(it);
            ++removed;
        }
        else
        {
            ++it;
        }
    }
}

uint64_t NotificationManager::NextId() noexcept
{
    static uint64_t s_next = 1;
    return s_next++;
}

} // namespace DragonOS::Notifications
