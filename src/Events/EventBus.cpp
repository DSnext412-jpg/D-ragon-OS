#include <Events/EventBus.hpp>

#include <algorithm>

namespace DragonOS::Events {

dragonos::sdk::EventHandlerId EventBus::Subscribe(
    dragonos::sdk::EventType type,
    dragonos::sdk::EventCallback callback,
    int priority) noexcept
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);

    HandlerEntry entry;
    entry.id = NextId();
    entry.type = type;
    entry.callback = std::move(callback);
    entry.priority = priority;
    m_handlers.push_back(std::move(entry));

    return entry.id;
}

dragonos::sdk::EventHandlerId EventBus::Subscribe(
    dragonos::sdk::EventType type,
    dragonos::sdk::IEventHandler* handler,
    int priority) noexcept
{
    if (!handler) { return 0; }

    std::lock_guard<std::mutex> lock(m_handlerMutex);

    HandlerEntry entry;
    entry.id = NextId();
    entry.type = type;
    entry.handler = handler;
    entry.priority = priority;
    m_handlers.push_back(std::move(entry));

    return entry.id;
}

bool EventBus::Unsubscribe(dragonos::sdk::EventHandlerId id) noexcept
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);

    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it)
    {
        if (it->id == id)
        {
            m_handlers.erase(it);
            return true;
        }
    }
    return false;
}

void EventBus::Publish(const dragonos::sdk::Event& event) noexcept
{
    std::lock_guard<std::mutex> lock(m_handlerMutex);

    std::sort(m_handlers.begin(), m_handlers.end(),
        [](const auto& a, const auto& b) { return a.priority > b.priority; });

    for (const auto& entry : m_handlers)
    {
        if (entry.type == event.type ||
            entry.type == dragonos::sdk::EventType::Custom)
        {
            if (entry.callback)
            {
                entry.callback(event);
            }
            else if (entry.handler)
            {
                entry.handler->OnEvent(event);
            }
        }
    }
}

void EventBus::PublishAsync(const dragonos::sdk::Event& event) noexcept
{
    std::lock_guard<std::mutex> lock(m_queueMutex);

    if (m_asyncQueue.size() < MaxQueuedEvents)
    {
        m_asyncQueue.push_back(event);
    }
}

void EventBus::ProcessAsyncEvents() noexcept
{
    std::vector<dragonos::sdk::Event> queue;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        queue.swap(m_asyncQueue);
    }

    for (const auto& event : queue)
    {
        Publish(event);
    }
}

dragonos::sdk::EventHandlerId EventBus::NextId() noexcept
{
    static dragonos::sdk::EventHandlerId s_next = 1;
    return s_next++;
}

} // namespace DragonOS::Events
