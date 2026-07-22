#pragma once

#include <Process/Process.hpp>
#include <Process/ProcessHandle.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Process {

class ProcessManager final : public Engine::System {
public:
    ProcessManager() noexcept = default;
    ~ProcessManager() noexcept { Shutdown(); }

    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;
    ProcessManager(ProcessManager&&) = delete;
    ProcessManager& operator=(ProcessManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    ProcessHandle SpawnProcess(std::wstring name, bool isBackground = false) noexcept;
    bool TerminateProcess(uint64_t processId) noexcept;
    bool SuspendProcess(uint64_t processId) noexcept;
    bool ResumeProcess(uint64_t processId) noexcept;

    Process* Find(uint64_t processId) noexcept;
    const Process* Find(uint64_t processId) const noexcept;

    std::vector<const Process*> GetAll() const noexcept;
    std::vector<const Process*> GetRunning() const noexcept;
    size_t GetCount() const noexcept { return m_processes.size(); }

private:
    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    void RemoveTerminated() noexcept;

    std::unordered_map<uint64_t, std::unique_ptr<Process>> m_processes;
    bool m_initialized{ false };
};

} // namespace DragonOS::Process
