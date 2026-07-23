#pragma once

#include <Diagnostics/LogManager.hpp>
#include <Diagnostics/PerformanceProfiler.hpp>
#include <Diagnostics/CrashReporter.hpp>
#include <Diagnostics/SystemMonitorWindow.hpp>
#include <Diagnostics/DebugConsoleWindow.hpp>
#include <Diagnostics/DiagnosticDashboardWindow.hpp>

#include <Engine/System.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; class MouseManager; }
namespace DragonOS::Animation { class AnimationManager; }
namespace DragonOS::WindowManager { class WindowManager; class DragonWindow; }
namespace DragonOS::Process { class ProcessManager; }
namespace DragonOS::AppRuntime { class ApplicationManager; }
namespace DragonOS::Notifications { class NotificationManager; }

namespace DragonOS::Diagnostics {

class DevToolsManager final : public Engine::System {
public:
    DevToolsManager(
        WindowManager::WindowManager& wndMgr,
        Theme::ThemeManager& themeMgr,
        Animation::AnimationManager& animMgr,
        Input::InputManager& inputMgr,
        Process::ProcessManager& procMgr,
        AppRuntime::ApplicationManager& appMgr,
        Notifications::NotificationManager& notifMgr) noexcept;

    ~DevToolsManager() noexcept override { Shutdown(); }

    DevToolsManager(const DevToolsManager&) = delete;
    DevToolsManager& operator=(const DevToolsManager&) = delete;
    DevToolsManager(DevToolsManager&&) = delete;
    DevToolsManager& operator=(DevToolsManager&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void SetMouseManager(Input::MouseManager& mouseMgr) noexcept
    {
        m_pMouse = &mouseMgr;
    }

    LogManager& GetLogManager() noexcept { return *m_pLogMgr; }
    PerformanceProfiler& GetProfiler() noexcept { return *m_pProfiler; }
    CrashReporter& GetCrashReporter() noexcept { return *m_pCrashReporter; }

    void OpenSystemMonitor() noexcept;
    void OpenDebugConsole() noexcept;
    void OpenDashboard() noexcept;

    void CloseSystemMonitor(uint64_t windowId) noexcept;
    void CloseDebugConsole(uint64_t windowId) noexcept;
    void CloseDashboard(uint64_t windowId) noexcept;

    [[nodiscard]] size_t GetSystemMonitorCount() const noexcept { return m_monitorInstances.size(); }
    [[nodiscard]] size_t GetDebugConsoleCount() const noexcept { return m_consoleInstances.size(); }
    [[nodiscard]] size_t GetDashboardCount() const noexcept { return m_dashboardInstances.size(); }

private:
    struct MonitorInstance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<SystemMonitorWindow> pContent;
    };

    struct ConsoleInstance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<DebugConsoleWindow> pContent;
    };

    struct DashboardInstance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<DiagnosticDashboardWindow> pContent;
    };

    void RemoveClosedWindows() noexcept;
    void WireConsoleToLogManager() noexcept;

    std::unique_ptr<LogManager> m_pLogMgr;
    std::unique_ptr<PerformanceProfiler> m_pProfiler;
    std::unique_ptr<CrashReporter> m_pCrashReporter;

    std::vector<MonitorInstance> m_monitorInstances;
    std::vector<ConsoleInstance> m_consoleInstances;
    std::vector<DashboardInstance> m_dashboardInstances;

    WindowManager::WindowManager& m_windowManager;
    Theme::ThemeManager& m_themeManager;
    Animation::AnimationManager& m_animManager;
    Input::InputManager& m_inputManager;
    Process::ProcessManager& m_procManager;
    AppRuntime::ApplicationManager& m_appManager;
    Notifications::NotificationManager& m_notifMgr;
    Input::MouseManager* m_pMouse{ nullptr };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
    float m_updateTimer{ 0.0f };
    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
