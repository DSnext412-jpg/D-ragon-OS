#pragma once

#include <Services/ServiceState.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>

namespace DragonOS::Services {

class ServiceManager;

class BackgroundService {
    friend class ServiceManager;
public:
    virtual ~BackgroundService() noexcept = default;

    virtual std::wstring_view GetName() const noexcept = 0;
    virtual std::wstring_view GetDisplayName() const noexcept = 0;
    virtual std::wstring_view GetDescription() const noexcept = 0;

    virtual bool Start() noexcept = 0;
    virtual bool Stop() noexcept = 0;
    virtual bool Pause() noexcept;
    virtual bool Resume() noexcept;

    virtual void Update(float deltaTime) noexcept;

    ServiceState GetState() const noexcept { return m_state; }
    bool IsRunning() const noexcept { return m_state == ServiceState::Running; }

    uint64_t GetId() const noexcept { return m_id; }
    std::chrono::steady_clock::time_point GetLastStartTime() const noexcept { return m_lastStart; }

protected:
    BackgroundService() noexcept = default;

    uint64_t m_id{ 0 };
    ServiceState m_state{ ServiceState::Stopped };
    std::chrono::steady_clock::time_point m_lastStart;
};

} // namespace DragonOS::Services
