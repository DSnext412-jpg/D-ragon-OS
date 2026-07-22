#include "NotificationServiceAdapter.hpp"

namespace DragonOS::SDK {

static Notifications::NotificationSeverity ToInternal(
    dragonos::sdk::NotificationSeverity sev) noexcept
{
    switch (sev)
    {
    case dragonos::sdk::NotificationSeverity::Information:
        return Notifications::NotificationSeverity::Information;
    case dragonos::sdk::NotificationSeverity::Warning:
        return Notifications::NotificationSeverity::Warning;
    case dragonos::sdk::NotificationSeverity::Error:
        return Notifications::NotificationSeverity::Error;
    case dragonos::sdk::NotificationSeverity::Success:
        return Notifications::NotificationSeverity::Success;
    }
    return Notifications::NotificationSeverity::Information;
}

uint64_t NotificationServiceAdapter::Show(
    const dragonos::sdk::NotificationData& data) noexcept
{
    Notifications::Notification n;
    n.title = data.title;
    n.message = data.message;
    n.source = data.source;
    n.severity = ToInternal(data.severity);
    n.dismissMode = Notifications::NotificationDismiss::Manual;
    return m_manager.Show(n);
}

bool NotificationServiceAdapter::Dismiss(uint64_t id) noexcept
{
    return m_manager.Dismiss(id);
}

bool NotificationServiceAdapter::DismissGroup(std::wstring_view groupKey) noexcept
{
    return m_manager.DismissGroup(std::wstring{ groupKey });
}

void NotificationServiceAdapter::DismissAll() noexcept
{
    m_manager.DismissAll();
}

} // namespace DragonOS::SDK
