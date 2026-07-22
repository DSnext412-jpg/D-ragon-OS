#pragma once

#include <DragonOS/Notification.hpp>

#include <Notifications/NotificationManager.hpp>

namespace DragonOS::SDK {

class NotificationServiceAdapter final : public dragonos::sdk::INotificationService {
public:
    explicit NotificationServiceAdapter(
        Notifications::NotificationManager& mgr) noexcept
        : m_manager{ mgr }
    {
    }

    uint64_t Show(const dragonos::sdk::NotificationData& data) noexcept override;
    bool Dismiss(uint64_t id) noexcept override;
    bool DismissGroup(std::wstring_view groupKey) noexcept override;
    void DismissAll() noexcept override;

private:
    Notifications::NotificationManager& m_manager;
};

} // namespace DragonOS::SDK
