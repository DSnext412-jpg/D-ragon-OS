#pragma once

#include <Notifications/Notification.hpp>

#include <Engine/System.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace DragonOS::Notifications {

class NotificationManager final : public Engine::System {
public:
    NotificationManager() noexcept = default;
    ~NotificationManager() noexcept override { Shutdown(); }

    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;
    NotificationManager(NotificationManager&&) = delete;
    NotificationManager& operator=(NotificationManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    uint64_t Show(Notification notification) noexcept;
    bool Dismiss(uint64_t id) noexcept;
    bool DismissGroup(std::wstring_view groupKey) noexcept;
    void DismissAll() noexcept;
    void MarkRead(uint64_t id) noexcept;

    const Notification* Find(uint64_t id) const noexcept;
    std::vector<const Notification*> GetActive() const noexcept;
    std::vector<const Notification*> GetHistory() const noexcept;
    std::vector<const Notification*> GetGrouped(std::wstring_view groupKey) const noexcept;

    size_t GetActiveCount() const noexcept { return m_activeCount; }
    bool HasUnread() const noexcept { return m_unreadCount > 0; }
    size_t GetUnreadCount() const noexcept { return m_unreadCount; }

    using NotificationCallback = std::function<void(const Notification&)>;
    void SetOnShowCallback(NotificationCallback cb) noexcept { m_onShow = std::move(cb); }
    void SetOnDismissCallback(NotificationCallback cb) noexcept { m_onDismiss = std::move(cb); }

    static constexpr size_t MaxHistory = 200;
    static constexpr float AutoDismissDuration = 6.0f;

private:
    uint64_t NextId() noexcept;
    void TrimHistory() noexcept;

    std::vector<Notification> m_notifications;
    size_t m_activeCount{ 0 };
    size_t m_unreadCount{ 0 };
    NotificationCallback m_onShow;
    NotificationCallback m_onDismiss;
    bool m_initialized{ false };
};

} // namespace DragonOS::Notifications
