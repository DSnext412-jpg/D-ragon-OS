#include "EventBusAdapter.hpp"

namespace DragonOS::SDK {

dragonos::sdk::EventHandlerId EventBusAdapter::Subscribe(
    dragonos::sdk::EventType type,
    dragonos::sdk::EventCallback callback,
    int priority) noexcept
{
    return m_bus.Subscribe(type, std::move(callback), priority);
}

dragonos::sdk::EventHandlerId EventBusAdapter::Subscribe(
    dragonos::sdk::EventType type,
    dragonos::sdk::IEventHandler* handler,
    int priority) noexcept
{
    return m_bus.Subscribe(type, handler, priority);
}

bool EventBusAdapter::Unsubscribe(
    dragonos::sdk::EventHandlerId id) noexcept
{
    return m_bus.Unsubscribe(id);
}

void EventBusAdapter::Publish(
    const dragonos::sdk::Event& event) noexcept
{
    m_bus.Publish(event);
}

void EventBusAdapter::PublishAsync(
    const dragonos::sdk::Event& event) noexcept
{
    m_bus.PublishAsync(event);
}

} // namespace DragonOS::SDK
