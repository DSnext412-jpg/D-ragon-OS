#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace dragonos::sdk {

enum class NotificationSeverity {
    Information,
    Warning,
    Error,
    Success,
    Progress,
};

enum class NotificationDismiss {
    Manual,
    Automatic,
    OnClick,
};

struct NotificationAction {
    std::wstring label;
    std::function<void()> callback;
};

struct NotificationData {
    uint64_t id{ 0 };
    std::wstring title;
    std::wstring message;
    std::wstring source;
    std::wstring groupKey;
    NotificationSeverity severity{ NotificationSeverity::Information };
    NotificationDismiss dismiss{ NotificationDismiss::Automatic };
    float progress{ 0.0f };
    std::vector<NotificationAction> actions;
};

class INotificationService {
public:
    virtual ~INotificationService() noexcept = default;
    virtual uint64_t Show(const NotificationData& data) noexcept = 0;
    virtual bool Dismiss(uint64_t id) noexcept = 0;
    virtual bool DismissGroup(std::wstring_view groupKey) noexcept = 0;
    virtual void DismissAll() noexcept = 0;
};

} // namespace dragonos::sdk
