#pragma once

#include <Services/BackgroundService.hpp>
#include <Services/ServiceState.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DragonOS::Services {

using BackgroundServicePtr = std::unique_ptr<BackgroundService>;

class ServiceManager final : public Engine::System {
public:
    ServiceManager() noexcept = default;
    ~ServiceManager() noexcept override { Shutdown(); }

    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;
    ServiceManager(ServiceManager&&) = delete;
    ServiceManager& operator=(ServiceManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    uint64_t Register(BackgroundServicePtr service) noexcept;

    bool Start(uint64_t serviceId) noexcept;
    bool Start(std::wstring_view name) noexcept;
    bool Stop(uint64_t serviceId) noexcept;
    bool Stop(std::wstring_view name) noexcept;
    bool Pause(uint64_t serviceId) noexcept;
    bool Resume(uint64_t serviceId) noexcept;

    BackgroundService* Find(uint64_t serviceId) noexcept;
    BackgroundService* Find(std::wstring_view name) noexcept;

    std::vector<BackgroundService*> GetAll() const noexcept;
    std::vector<BackgroundService*> GetRunning() const noexcept;
    std::vector<BackgroundService*> GetByState(ServiceState state) const noexcept;

    size_t GetCount() const noexcept { return m_services.size(); }
    size_t GetRunningCount() const noexcept;
    bool HasActiveServices() const noexcept;

    void StartAll() noexcept;
    void StopAll() noexcept;

    static constexpr float HealthCheckInterval = 5.0f;

private:
    uint64_t NextId() noexcept;

    struct ServiceEntry {
        BackgroundServicePtr service;
        float healthTimer{ 0.0f };
    };

    std::unordered_map<uint64_t, ServiceEntry> m_services;
    bool m_initialized{ false };
};

} // namespace DragonOS::Services
