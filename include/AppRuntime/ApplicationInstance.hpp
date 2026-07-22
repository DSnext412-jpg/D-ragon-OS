#pragma once

#include <AppRuntime/ApplicationMetadata.hpp>

#include <cstdint>
#include <string>

namespace DragonOS::WindowManager { class DragonWindow; }

namespace DragonOS::AppRuntime {

class ApplicationInstance final {
public:
    ApplicationInstance(
        ApplicationMetadata metadata,
        WindowManager::DragonWindow* pWindow = nullptr) noexcept
        : m_metadata{ std::move(metadata) }
        , m_pWindow{ pWindow }
    {
    }

    [[nodiscard]] const ApplicationMetadata& GetMetadata() const noexcept { return m_metadata; }

    [[nodiscard]] uint64_t GetAppId() const noexcept { return m_metadata.appId; }
    [[nodiscard]] uint64_t GetRegistryId() const noexcept { return m_metadata.registryId; }
    [[nodiscard]] const std::wstring& GetName() const noexcept { return m_metadata.name; }
    [[nodiscard]] const std::wstring& GetDisplayName() const noexcept { return m_metadata.displayName; }
    [[nodiscard]] ApplicationState GetState() const noexcept { return m_metadata.state; }
    [[nodiscard]] uint64_t GetLaunchTime() const noexcept { return m_metadata.launchTime; }

    [[nodiscard]] WindowManager::DragonWindow* GetWindow() const noexcept { return m_pWindow; }
    void SetWindow(WindowManager::DragonWindow* pWindow) noexcept { m_pWindow = pWindow; }

    void SetState(ApplicationState state) noexcept { m_metadata.state = state; }

    void Suspend() noexcept { m_metadata.state = ApplicationState::Suspended; }
    void Resume() noexcept { m_metadata.state = ApplicationState::Running; }
    void Terminate() noexcept { m_metadata.state = ApplicationState::Terminated; }

private:
    ApplicationMetadata            m_metadata;
    WindowManager::DragonWindow*   m_pWindow{ nullptr };
};

} // namespace DragonOS::AppRuntime
