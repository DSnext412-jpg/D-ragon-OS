#include <AppRuntime/ApplicationManager.hpp>

#include <Windows.h>
#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::AppRuntime {

bool ApplicationManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void ApplicationManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }
    m_instances.clear();
    m_initialized = false;
}

void ApplicationManager::Update(float /*deltaTime*/) noexcept
{
    if (!m_initialized) { return; }
    RemoveTerminated();
}

void ApplicationManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void ApplicationManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

uint64_t ApplicationManager::RegisterApplication(
    uint64_t registryId,
    const std::wstring& name,
    const std::wstring& displayName,
    WindowManager::DragonWindow* pWindow) noexcept
{
    if (!m_initialized) { return 0; }

    ApplicationMetadata meta;
    meta.appId = NextId();
    meta.registryId = registryId;
    meta.name = name;
    meta.displayName = displayName;
    meta.state = ApplicationState::Running;
    meta.launchTime = static_cast<uint64_t>(::GetTickCount64());
    meta.isSystem = false;

    auto instance = std::make_unique<ApplicationInstance>(std::move(meta), pWindow);
    const uint64_t id = instance->GetAppId();
    m_instances[id] = std::move(instance);
    return id;
}

bool ApplicationManager::UnregisterApplication(uint64_t appId) noexcept
{
    auto it = m_instances.find(appId);
    if (it == m_instances.end()) { return false; }
    m_instances.erase(it);
    return true;
}

ApplicationInstance* ApplicationManager::Find(uint64_t appId) noexcept
{
    auto it = m_instances.find(appId);
    return (it != m_instances.end()) ? it->second.get() : nullptr;
}

const ApplicationInstance* ApplicationManager::Find(uint64_t appId) const noexcept
{
    auto it = m_instances.find(appId);
    return (it != m_instances.end()) ? it->second.get() : nullptr;
}

ApplicationInstance* ApplicationManager::FindByWindow(uint64_t windowId) noexcept
{
    for (auto& [id, inst] : m_instances)
    {
        if (inst->GetWindow() && inst->GetWindow()->GetId() == windowId)
        {
            return inst.get();
        }
    }
    return nullptr;
}

size_t ApplicationManager::GetRunningCount() const noexcept
{
    size_t count = 0;
    for (const auto& [id, inst] : m_instances)
    {
        if (inst->GetState() == ApplicationState::Running)
        {
            ++count;
        }
    }
    return count;
}

std::vector<const ApplicationInstance*> ApplicationManager::GetAll() const noexcept
{
    std::vector<const ApplicationInstance*> result;
    result.reserve(m_instances.size());
    for (const auto& [id, inst] : m_instances)
    {
        result.push_back(inst.get());
    }
    return result;
}

std::vector<const ApplicationInstance*> ApplicationManager::GetRunning() const noexcept
{
    std::vector<const ApplicationInstance*> result;
    result.reserve(m_instances.size());
    for (const auto& [id, inst] : m_instances)
    {
        if (inst->GetState() == ApplicationState::Running)
        {
            result.push_back(inst.get());
        }
    }
    return result;
}

bool ApplicationManager::SuspendApplication(uint64_t appId) noexcept
{
    auto* inst = Find(appId);
    if (!inst) { return false; }
    inst->Suspend();
    return true;
}

bool ApplicationManager::ResumeApplication(uint64_t appId) noexcept
{
    auto* inst = Find(appId);
    if (!inst) { return false; }
    inst->Resume();
    return true;
}

bool ApplicationManager::TerminateApplication(uint64_t appId) noexcept
{
    auto* inst = Find(appId);
    if (!inst) { return false; }
    inst->Terminate();
    return true;
}

void ApplicationManager::RemoveTerminated() noexcept
{
    for (auto it = m_instances.begin(); it != m_instances.end(); )
    {
        if (it->second->GetState() == ApplicationState::Terminated)
        {
            it = m_instances.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace DragonOS::AppRuntime
