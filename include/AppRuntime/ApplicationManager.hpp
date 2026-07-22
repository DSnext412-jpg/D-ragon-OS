#pragma once

#include <AppRuntime/ApplicationInstance.hpp>
#include <AppRuntime/ApplicationMetadata.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::AppRuntime {

class ApplicationManager final : public Engine::System {
public:
    ApplicationManager() noexcept = default;
    ~ApplicationManager() noexcept { Shutdown(); }

    ApplicationManager(const ApplicationManager&) = delete;
    ApplicationManager& operator=(const ApplicationManager&) = delete;
    ApplicationManager(ApplicationManager&&) = delete;
    ApplicationManager& operator=(ApplicationManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    uint64_t RegisterApplication(
        uint64_t registryId,
        const std::wstring& name,
        const std::wstring& displayName,
        WindowManager::DragonWindow* pWindow = nullptr) noexcept;

    bool UnregisterApplication(uint64_t appId) noexcept;

    ApplicationInstance* Find(uint64_t appId) noexcept;
    const ApplicationInstance* Find(uint64_t appId) const noexcept;

    ApplicationInstance* FindByWindow(uint64_t windowId) noexcept;

    size_t GetRunningCount() const noexcept;
    size_t GetTotalCount() const noexcept { return m_instances.size(); }

    std::vector<const ApplicationInstance*> GetAll() const noexcept;
    std::vector<const ApplicationInstance*> GetRunning() const noexcept;

    bool SuspendApplication(uint64_t appId) noexcept;
    bool ResumeApplication(uint64_t appId) noexcept;
    bool TerminateApplication(uint64_t appId) noexcept;

private:
    uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    void RemoveTerminated() noexcept;

    std::unordered_map<uint64_t, std::unique_ptr<ApplicationInstance>> m_instances;
    bool m_initialized{ false };
};

} // namespace DragonOS::AppRuntime
