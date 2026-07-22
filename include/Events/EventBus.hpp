#pragma once

#include <DragonOS/Events.hpp>

#include <Engine/System.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Events {

class EventBus final : public dragonos::sdk::IEventBus {
public:
    EventBus() noexcept = default;
    ~EventBus() noexcept override = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    dragonos::sdk::EventHandlerId Subscribe(
        dragonos::sdk::EventType type,
        dragonos::sdk::EventCallback callback,
        int priority = 0) noexcept override;

    dragonos::sdk::EventHandlerId Subscribe(
        dragonos::sdk::EventType type,
        dragonos::sdk::IEventHandler* handler,
        int priority = 0) noexcept override;

    bool Unsubscribe(dragonos::sdk::EventHandlerId id) noexcept override;

    void Publish(const dragonos::sdk::Event& event) noexcept override;
    void PublishAsync(const dragonos::sdk::Event& event) noexcept override;

    void ProcessAsyncEvents() noexcept;

    static constexpr size_t MaxQueuedEvents = 200;

private:
    struct HandlerEntry {
        dragonos::sdk::EventHandlerId id{ 0 };
        dragonos::sdk::EventType type;
        dragonos::sdk::EventCallback callback;
        dragonos::sdk::IEventHandler* handler{ nullptr };
        int priority{ 0 };
    };

    dragonos::sdk::EventHandlerId NextId() noexcept;

    std::vector<HandlerEntry> m_handlers;
    std::mutex m_handlerMutex;

    std::vector<dragonos::sdk::Event> m_asyncQueue;
    std::mutex m_queueMutex;
};

} // namespace DragonOS::Events
