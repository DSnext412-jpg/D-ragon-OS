#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace dragonos::sdk {

enum class EventType : uint32_t {
    AppLaunched,
    AppClosed,
    WindowOpened,
    WindowClosed,
    WindowFocused,
    NotificationShown,
    NotificationDismissed,
    ThemeChanged,
    SessionSaving,
    SessionRestored,
    FileCreated,
    FileDeleted,
    FileModified,
    SearchPerformed,
    PluginLoaded,
    PluginUnloaded,
    Custom = 0x10000,
};

struct Event {
    EventType type{ EventType::Custom };
    uint64_t sourceId{ 0 };
    std::wstring sourceName;
    std::shared_ptr<void> data;
};

class IEventHandler {
public:
    virtual ~IEventHandler() noexcept = default;
    virtual void OnEvent(const Event& event) noexcept = 0;
};

using EventCallback = std::function<void(const Event&)>;
using EventHandlerId = uint64_t;

class IEventBus {
public:
    virtual ~IEventBus() noexcept = default;

    virtual EventHandlerId Subscribe(
        EventType type,
        EventCallback callback,
        int priority = 0) noexcept = 0;

    virtual EventHandlerId Subscribe(
        EventType type,
        IEventHandler* handler,
        int priority = 0) noexcept = 0;

    virtual bool Unsubscribe(EventHandlerId id) noexcept = 0;
    virtual void Publish(const Event& event) noexcept = 0;
    virtual void PublishAsync(const Event& event) noexcept = 0;
};

} // namespace dragonos::sdk
