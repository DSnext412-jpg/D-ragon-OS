#include <Process/ProcessManager.hpp>

namespace DragonOS::Process {

bool ProcessManager::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void ProcessManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }
    m_processes.clear();
    m_initialized = false;
}

void ProcessManager::Update(float /*deltaTime*/) noexcept
{
    if (!m_initialized) { return; }
    RemoveTerminated();
}

void ProcessManager::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void ProcessManager::Resize(float /*width*/, float /*height*/) noexcept
{
}

ProcessHandle ProcessManager::SpawnProcess(
    std::wstring name,
    bool isBackground) noexcept
{
    if (!m_initialized) { return ProcessHandle{}; }

    const uint64_t id = NextId();
    auto proc = std::make_unique<Process>(id, std::move(name), isBackground);
    ProcessHandle handle{ id };
    m_processes[id] = std::move(proc);
    return handle;
}

bool ProcessManager::TerminateProcess(uint64_t processId) noexcept
{
    auto* proc = Find(processId);
    if (!proc) { return false; }
    proc->Terminate();
    return true;
}

bool ProcessManager::SuspendProcess(uint64_t processId) noexcept
{
    auto* proc = Find(processId);
    if (!proc) { return false; }
    proc->Suspend();
    return true;
}

bool ProcessManager::ResumeProcess(uint64_t processId) noexcept
{
    auto* proc = Find(processId);
    if (!proc) { return false; }
    proc->Resume();
    return true;
}

Process* ProcessManager::Find(uint64_t processId) noexcept
{
    auto it = m_processes.find(processId);
    return (it != m_processes.end()) ? it->second.get() : nullptr;
}

const Process* ProcessManager::Find(uint64_t processId) const noexcept
{
    auto it = m_processes.find(processId);
    return (it != m_processes.end()) ? it->second.get() : nullptr;
}

std::vector<const Process*> ProcessManager::GetAll() const noexcept
{
    std::vector<const Process*> result;
    result.reserve(m_processes.size());
    for (const auto& [id, proc] : m_processes)
    {
        result.push_back(proc.get());
    }
    return result;
}

std::vector<const Process*> ProcessManager::GetRunning() const noexcept
{
    std::vector<const Process*> result;
    result.reserve(m_processes.size());
    for (const auto& [id, proc] : m_processes)
    {
        if (proc->GetState() == ProcessState::Running ||
            proc->GetState() == ProcessState::Background)
        {
            result.push_back(proc.get());
        }
    }
    return result;
}

void ProcessManager::RemoveTerminated() noexcept
{
    for (auto it = m_processes.begin(); it != m_processes.end(); )
    {
        if (it->second->GetState() == ProcessState::Terminated)
        {
            it = m_processes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace DragonOS::Process
