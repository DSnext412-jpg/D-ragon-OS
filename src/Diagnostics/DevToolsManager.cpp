#include <Diagnostics/DevToolsManager.hpp>

#include <algorithm>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Input/InputManager.hpp>
#include <WindowManager/WindowManager.hpp>
#include <Process/ProcessManager.hpp>
#include <AppRuntime/ApplicationManager.hpp>
#include <Notifications/NotificationManager.hpp>

namespace DragonOS::Diagnostics {

DevToolsManager::DevToolsManager(
    WindowManager::WindowManager& wndMgr,
    Theme::ThemeManager& themeMgr,
    Animation::AnimationManager& animMgr,
    Input::InputManager& inputMgr,
    Process::ProcessManager& procMgr,
    AppRuntime::ApplicationManager& appMgr,
    Notifications::NotificationManager& notifMgr) noexcept
    : m_windowManager{ wndMgr }
    , m_themeManager{ themeMgr }
    , m_animManager{ animMgr }
    , m_inputManager{ inputMgr }
    , m_procManager{ procMgr }
    , m_appManager{ appMgr }
    , m_notifMgr{ notifMgr }
{
}

bool DevToolsManager::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;

    m_viewportWidth = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_pLogMgr = std::make_unique<LogManager>();
    m_pProfiler = std::make_unique<PerformanceProfiler>();
    m_pCrashReporter = std::make_unique<CrashReporter>();

    m_pLogMgr->Initialize(ctx);
    m_pProfiler->Initialize(ctx);
    m_pCrashReporter->Initialize(ctx);

    m_pCrashReporter->SetLogManager(*m_pLogMgr);
    m_pCrashReporter->SetNotificationManager(m_notifMgr);

    m_initialized = true;
    return true;
}

void DevToolsManager::Shutdown() noexcept
{
    if (!m_initialized) return;

    for (auto& inst : m_monitorInstances)
    {
        if (inst.pWindow)
            m_windowManager.RemoveWindow(inst.pWindow);
    }
    m_monitorInstances.clear();

    for (auto& inst : m_consoleInstances)
    {
        if (inst.pWindow)
            m_windowManager.RemoveWindow(inst.pWindow);
    }
    m_consoleInstances.clear();

    for (auto& inst : m_dashboardInstances)
    {
        if (inst.pWindow)
            m_windowManager.RemoveWindow(inst.pWindow);
    }
    m_dashboardInstances.clear();

    m_pCrashReporter->Shutdown();
    m_pProfiler->Shutdown();
    m_pLogMgr->Shutdown();

    m_pCrashReporter.reset();
    m_pProfiler.reset();
    m_pLogMgr.reset();

    m_initialized = false;
}

void DevToolsManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) return;

    m_updateTimer += deltaTime;

    m_pLogMgr->Update(deltaTime);
    m_pProfiler->Update(deltaTime);
    m_pCrashReporter->Update(deltaTime);

    RemoveClosedWindows();

    if (m_updateTimer >= 0.5f)
    {
        m_updateTimer = 0.0f;
        auto processes = m_procManager.GetAll();
        auto apps = m_appManager.GetAll();

        double cpuEstimate = static_cast<double>(processes.size()) * 5.0;
        if (cpuEstimate > 95.0) cpuEstimate = 95.0;

        double memMB = static_cast<double>(processes.size()) * 32.0;
        double totalMB = 8192.0;

        m_pProfiler->SetCpuUsage(cpuEstimate);
        m_pProfiler->RecordMetric(MetricType::CpuUsage, cpuEstimate);
        m_pProfiler->RecordMetric(MetricType::MemoryUsage, (memMB / totalMB) * 100.0);
        m_pProfiler->RecordMetric(MetricType::ProcessCount, static_cast<double>(processes.size()));

        DiagnosticSummary summary;
        summary.cpuUsagePercent = cpuEstimate;
        summary.memoryUsagePercent = (memMB / totalMB) * 100.0;
        summary.memoryUsedMB = memMB;
        summary.memoryTotalMB = totalMB;
        summary.processCount = processes.size();
        summary.runningAppCount = apps.size();
        summary.activeNotificationCount = m_notifMgr.GetActiveCount();
        summary.unreadNotificationCount = m_notifMgr.GetUnreadCount();
        summary.unreadLogCount = m_pLogMgr->GetUnreadCount();
        summary.securityEnabled = true;

        for (auto& inst : m_monitorInstances)
        {
            if (inst.pContent)
            {
                inst.pContent->UpdateMetrics(
                    cpuEstimate, memMB, totalMB, processes.size());
            }
        }

        for (auto& inst : m_dashboardInstances)
        {
            if (inst.pContent)
            {
                inst.pContent->UpdateSummary(summary);
            }
        }
    }

    for (auto& inst : m_monitorInstances)
    {
        if (inst.pContent) inst.pContent->Update();
    }

    for (auto& inst : m_consoleInstances)
    {
        if (inst.pContent)
        {
            inst.pContent->Update();
            if (inst.pWindow && inst.pWindow->IsFocused())
            {
                inst.pContent->ProcessKeyboardInput(m_inputManager);
            }
        }
    }

    for (auto& inst : m_dashboardInstances)
    {
        if (inst.pContent) inst.pContent->Update();
    }
}

void DevToolsManager::Render(Engine::EngineContext& ctx) noexcept
{
    if (!m_initialized) return;

    auto* renderer = ctx.GetRenderer();
    if (!renderer) return;
}

void DevToolsManager::Resize(float width, float height) noexcept
{
    m_viewportWidth = width;
    m_viewportHeight = height;
}

void DevToolsManager::OpenSystemMonitor() noexcept
{
    auto content = std::make_unique<SystemMonitorWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"System Monitor", 100.0f, 100.0f, 480.0f, 360.0f);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) return;

    pContent->SetDependencies(*pWindow, m_themeManager, *m_pMouse, m_procManager);

    MonitorInstance inst;
    inst.pWindow = pWindow;
    inst.pContent = std::move(content);
    m_monitorInstances.push_back(std::move(inst));

    m_windowManager.SetFocusedWindow(pWindow);

    auto processes = m_procManager.GetAll();
    pContent->UpdateMetrics(0.0, 0.0, 8192.0, processes.size());
}

void DevToolsManager::OpenDebugConsole() noexcept
{
    auto content = std::make_unique<DebugConsoleWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"Debug Console", 150.0f, 150.0f, 600.0f, 400.0f);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) return;

    pContent->SetDependencies(*pWindow, m_themeManager, m_inputManager, *m_pMouse);

    WireConsoleToLogManager();

    ConsoleInstance inst;
    inst.pWindow = pWindow;
    inst.pContent = std::move(content);
    m_consoleInstances.push_back(std::move(inst));

    m_windowManager.SetFocusedWindow(pWindow);
}

void DevToolsManager::OpenDashboard() noexcept
{
    float vpW = m_viewportWidth;
    float vpH = m_viewportHeight;
    float w = 520.0f;
    float h = 400.0f;
    float x = (vpW - w) / 2.0f;
    float y = (vpH - h) / 2.0f;

    auto content = std::make_unique<DiagnosticDashboardWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"Diagnostic Dashboard", x, y, w, h);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) return;

    pContent->SetDependencies(*pWindow, m_themeManager, *m_pMouse);

    pContent->SetQuickActionCallback(L"System Monitor", [this]()
    {
        OpenSystemMonitor();
    });

    pContent->SetQuickActionCallback(L"Debug Console", [this]()
    {
        OpenDebugConsole();
    });

    pContent->SetQuickActionCallback(L"Clear Logs", [this]()
    {
        m_pLogMgr->Clear();
    });

    pContent->SetQuickActionCallback(L"Refresh", [this]()
    {
    });

    DashboardInstance inst;
    inst.pWindow = pWindow;
    inst.pContent = std::move(content);
    m_dashboardInstances.push_back(std::move(inst));

    m_windowManager.SetFocusedWindow(pWindow);

    auto processes = m_procManager.GetAll();
    DiagnosticSummary summary;
    summary.processCount = processes.size();
    summary.runningAppCount = m_appManager.GetAll().size();
    summary.activeNotificationCount = m_notifMgr.GetActiveCount();
    summary.unreadNotificationCount = m_notifMgr.GetUnreadCount();
    summary.unreadLogCount = m_pLogMgr->GetUnreadCount();
    summary.securityEnabled = true;
    pContent->UpdateSummary(summary);
}

void DevToolsManager::CloseSystemMonitor(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_monitorInstances.begin(), m_monitorInstances.end(),
        [windowId](const MonitorInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_monitorInstances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_monitorInstances.erase(it);
    }
}

void DevToolsManager::CloseDebugConsole(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_consoleInstances.begin(), m_consoleInstances.end(),
        [windowId](const ConsoleInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_consoleInstances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_consoleInstances.erase(it);
    }
}

void DevToolsManager::CloseDashboard(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_dashboardInstances.begin(), m_dashboardInstances.end(),
        [windowId](const DashboardInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_dashboardInstances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_dashboardInstances.erase(it);
    }
}

void DevToolsManager::RemoveClosedWindows() noexcept
{
    for (auto it = m_monitorInstances.begin(); it != m_monitorInstances.end(); )
    {
        if (!it->pWindow || !it->pWindow->IsVisible())
        {
            m_windowManager.RemoveWindow(it->pWindow);
            it = m_monitorInstances.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = m_consoleInstances.begin(); it != m_consoleInstances.end(); )
    {
        if (!it->pWindow || !it->pWindow->IsVisible())
        {
            m_windowManager.RemoveWindow(it->pWindow);
            it = m_consoleInstances.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = m_dashboardInstances.begin(); it != m_dashboardInstances.end(); )
    {
        if (!it->pWindow || !it->pWindow->IsVisible())
        {
            m_windowManager.RemoveWindow(it->pWindow);
            it = m_dashboardInstances.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DevToolsManager::WireConsoleToLogManager() noexcept
{
    m_pLogMgr->SetOnLogCallback([this](const LogEntry& entry)
    {
        for (auto& inst : m_consoleInstances)
        {
            if (inst.pContent)
            {
                inst.pContent->AppendLog(entry);
            }
        }
    });
}

} // namespace DragonOS::Diagnostics
