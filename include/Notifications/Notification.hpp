#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace DragonOS::Notifications {

enum class NotificationSeverity : uint8_t {
    Information,
    Warning,
    Error,
    Success,
    Progress,
};

enum class NotificationDismiss : uint8_t {
    Manual,
    Automatic,
    OnClick,
};

struct NotificationAction {
    std::wstring label;
    std::function<void()> callback;
};

struct Notification final {
    uint64_t id{ 0 };
    std::wstring title;
    std::wstring message;
    std::wstring source;          ///< App or system that created it.
    std::wstring groupKey;        ///< Grouping key (e.g. "updates", "network").
    NotificationSeverity severity{ NotificationSeverity::Information };
    NotificationDismiss dismissMode{ NotificationDismiss::Automatic };
    std::chrono::steady_clock::time_point timestamp;
    float progress{ 0.0f };       ///< 0..1 for Progress severity.
    bool dismissed{ false };
    bool read{ false };
    std::vector<NotificationAction> actions;
};

} // namespace DragonOS::Notifications
