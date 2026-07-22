#pragma once

#include <DragonOS/Events.hpp>

#include <Events/EventBus.hpp>

namespace DragonOS::SDK {

class EventBusAdapter final : public dragonos::sdk::IEventBus {
public:
    explicit EventBusAdapter(
        Events::EventBus& bus) noexcept
        : m_bus{ bus }
    {
    }

    dragonos::sdk::EventHandlerId Subscribe(
        dragonos::sdk::EventType type,
        dragonos::sdk::EventCallback callback,
        int priority = 0) noexcept override;
    dragonos::sdk::EventHandlerId Subscribe(
        dragonos::sdk::EventType type,
        dragonos::sdk::IEventHandler* handler,
        int priority = 0) noexcept override;
    bool Unsubscribe(
        dragonos::sdk::EventHandlerId id) noexcept override;
    void Publish(
        const dragonos::sdk::Event& event) noexcept override;
    void PublishAsync(
        const dragonos::sdk::Event& event) noexcept override;

private:
    Events::EventBus& m_bus;
};

} // namespace DragonOS::SDK
