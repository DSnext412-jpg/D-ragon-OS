#include <Services/ServiceManager.hpp>

#include <algorithm>

namespace DragonOS::Services {

bool ServiceManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void ServiceManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }
    StopAll();
    m_services.clear();
    m_initialized = false;
}

void ServiceManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    for (auto& [id, entry] : m_services)
    {
        if (entry.service->GetState() == ServiceState::Running)
        {
            entry.service->Update(deltaTime);
        }

        entry.healthTimer += deltaTime;
        if (entry.healthTimer >= HealthCheckInterval)
        {
            entry.healthTimer = 0.0f;
        }
    }
}

void ServiceManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void ServiceManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

uint64_t ServiceManager::Register(BackgroundServicePtr service) noexcept
{
    if (!service) { return 0; }
    const uint64_t id = NextId();
    service->m_id = id;
    ServiceEntry entry{ std::move(service), 0.0f };
    m_services[id] = std::move(entry);
    return id;
}

bool ServiceManager::Start(uint64_t serviceId) noexcept
{
    auto it = m_services.find(serviceId);
    if (it == m_services.end()) { return false; }
    return it->second.service->Start();
}

bool ServiceManager::Start(std::wstring_view name) noexcept
{
    for (auto& [id, entry] : m_services)
    {
        if (entry.service->GetName() == name)
        {
            return entry.service->Start();
        }
    }
    return false;
}

bool ServiceManager::Stop(uint64_t serviceId) noexcept
{
    auto it = m_services.find(serviceId);
    if (it == m_services.end()) { return false; }
    return it->second.service->Stop();
}

bool ServiceManager::Stop(std::wstring_view name) noexcept
{
    for (auto& [id, entry] : m_services)
    {
        if (entry.service->GetName() == name)
        {
            return entry.service->Stop();
        }
    }
    return false;
}

bool ServiceManager::Pause(uint64_t serviceId) noexcept
{
    auto it = m_services.find(serviceId);
    if (it == m_services.end()) { return false; }
    return it->second.service->Pause();
}

bool ServiceManager::Resume(uint64_t serviceId) noexcept
{
    auto it = m_services.find(serviceId);
    if (it == m_services.end()) { return false; }
    return it->second.service->Resume();
}

BackgroundService* ServiceManager::Find(uint64_t serviceId) noexcept
{
    auto it = m_services.find(serviceId);
    return (it != m_services.end()) ? it->second.service.get() : nullptr;
}

BackgroundService* ServiceManager::Find(std::wstring_view name) noexcept
{
    for (auto& [id, entry] : m_services)
    {
        if (entry.service->GetName() == name)
        {
            return entry.service.get();
        }
    }
    return nullptr;
}

std::vector<BackgroundService*> ServiceManager::GetAll() const noexcept
{
    std::vector<BackgroundService*> result;
    result.reserve(m_services.size());
    for (const auto& [id, entry] : m_services)
    {
        result.push_back(entry.service.get());
    }
    return result;
}

std::vector<BackgroundService*> ServiceManager::GetRunning() const noexcept
{
    return GetByState(ServiceState::Running);
}

std::vector<BackgroundService*> ServiceManager::GetByState(ServiceState state) const noexcept
{
    std::vector<BackgroundService*> result;
    for (const auto& [id, entry] : m_services)
    {
        if (entry.service->GetState() == state)
        {
            result.push_back(entry.service.get());
        }
    }
    return result;
}

size_t ServiceManager::GetRunningCount() const noexcept
{
    size_t count = 0;
    for (const auto& [id, entry] : m_services)
    {
        if (entry.service->GetState() == ServiceState::Running)
        {
            ++count;
        }
    }
    return count;
}

bool ServiceManager::HasActiveServices() const noexcept
{
    for (const auto& [id, entry] : m_services)
    {
        const auto state = entry.service->GetState();
        if (state == ServiceState::Running || state == ServiceState::Starting)
        {
            return true;
        }
    }
    return false;
}

void ServiceManager::StartAll() noexcept
{
    for (auto& [id, entry] : m_services)
    {
        entry.service->Start();
    }
}

void ServiceManager::StopAll() noexcept
{
    for (auto& [id, entry] : m_services)
    {
        entry.service->Stop();
    }
}

uint64_t ServiceManager::NextId() noexcept
{
    static uint64_t s_next = 1;
    return s_next++;
}

} // namespace DragonOS::Services

// ── BackgroundService default implementations ──────────────────────────────

namespace DragonOS::Services {

bool BackgroundService::Pause() noexcept
{
    if (m_state != ServiceState::Running) { return false; }
    m_state = ServiceState::Paused;
    return true;
}

bool BackgroundService::Resume() noexcept
{
    if (m_state != ServiceState::Paused) { return false; }
    m_state = ServiceState::Running;
    return true;
}

void BackgroundService::Update(float /*deltaTime*/) noexcept
{
}

} // namespace DragonOS::Services
