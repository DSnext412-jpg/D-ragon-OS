/**
 * @file    Engine.cpp
 * @brief   Implementation of the Engine class and its concrete Systems.
 *
 * This file also defines the concrete System subclasses that wrap the
 * existing DragonOS subsystems (Renderer, DesktopManager,
 * WindowManager) so they can be registered with SystemManager.
 */

#include <Engine/Engine.hpp>
#include <Engine/EngineContext.hpp>
#include <Engine/System.hpp>
#include <Engine/SystemManager.hpp>

#include <Animation/AnimationManager.hpp>
#include <AppRuntime/ApplicationManager.hpp>
#include <Apps/ApplicationRegistry.hpp>
#include <Desktop/DesktopManager.hpp>
#include <Explorer/ExplorerSystem.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/DebugOverlay.hpp>
#include <Input/InputManager.hpp>
#include <Process/ProcessManager.hpp>
#include <StartMenu/StartMenuController.hpp>
#include <Taskbar/Taskbar.hpp>
#include <Notifications/NotificationManager.hpp>
#include <Notifications/NotificationCenter.hpp>
#include <Search/SearchService.hpp>
#include <Search/SearchProvider.hpp>
#include <Search/SearchResult.hpp>
#include <Services/ServiceManager.hpp>
#include <Session/SessionManager.hpp>
#include <Terminal/TerminalSystem.hpp>
#include <Theme/ThemeManager.hpp>
#include <WindowManager/WindowManager.hpp>

#include <Plugins/PluginManager.hpp>
#include <ExtensionPoints/ExtensionPoint.hpp>
#include <Events/EventBus.hpp>
#include <Security/SecuritySystem.hpp>
#include <Diagnostics/DevToolsManager.hpp>

#include <SDK/NotificationServiceAdapter.hpp>
#include <SDK/ConfigServiceAdapter.hpp>
#include <SDK/FileServiceAdapter.hpp>
#include <SDK/ThemeServiceAdapter.hpp>
#include <SDK/InputServiceAdapter.hpp>
#include <SDK/ResourceServiceAdapter.hpp>
#include <SDK/WindowServiceAdapter.hpp>
#include <SDK/MenuServiceAdapter.hpp>
#include <SDK/DialogServiceAdapter.hpp>
#include <SDK/EventBusAdapter.hpp>

// ============================================================================
//  Concrete System implementations
// ============================================================================

namespace DragonOS::Engine {
namespace {

// ── DesktopSystem ─────────────────────────────────────────────────────────

/**
 * @brief  System wrapper for Desktop::DesktopManager.
 *
 * Renders the desktop background (wallpaper) as the first layer.
 */
class DesktopSystem final : public System {
public:
    explicit DesktopSystem(Desktop::DesktopManager& mgr) noexcept
        : m_manager{ mgr }
    {}

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();
        return true;
    }

    void Shutdown() noexcept override {}

    void Update(float /*deltaTime*/) noexcept override {}

    void Render(EngineContext& ctx) noexcept override
    {
        auto* renderer = ctx.GetRenderer();
        if (!renderer) { return; }

        m_manager.Render(
            *renderer,
            m_viewportWidth,
            m_viewportHeight);
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_manager.Resize(width, height);
    }

private:
    Desktop::DesktopManager& m_manager;
    float                    m_viewportWidth{ 0.0f };
    float                    m_viewportHeight{ 0.0f };
};

// ── WindowManagerSystem ───────────────────────────────────────────────────

/**
 * @brief  System wrapper for WindowManager::WindowManager.
 *
 * Renders all DragonWindows on top of the desktop background.
 */
class WindowManagerSystem final : public System {
public:
    explicit WindowManagerSystem(WindowManager::WindowManager& mgr) noexcept
        : m_manager{ mgr }
    {}

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();
        return true;
    }

    void Shutdown() noexcept override {}

    void Update(float deltaTime) noexcept override
    {
        m_manager.Update(deltaTime);
    }

    void Render(EngineContext& ctx) noexcept override
    {
        auto* renderer = ctx.GetRenderer();
        if (!renderer) { return; }

        m_manager.Render(*renderer, m_viewportWidth, m_viewportHeight);
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_manager.Resize(width, height);
    }

private:
    WindowManager::WindowManager& m_manager;
    float                         m_viewportWidth{ 0.0f };
    float                         m_viewportHeight{ 0.0f };
};

// ── TaskbarSystem ─────────────────────────────────────────────────────────

/**
 * @brief  System wrapper for Taskbar::Taskbar.
 *
 * Renders the taskbar on top of everything except the debug overlay.
 * Handles per-frame input processing and window state tracking.
 */
class TaskbarSystem final : public System {
public:
    explicit TaskbarSystem(
        WindowManager::WindowManager& wndMgr,
        Input::InputManager&          inputMgr,
        Theme::ThemeManager&          themeMgr,
        Animation::AnimationManager&  animMgr) noexcept
        : m_windowManager{ wndMgr }
        , m_inputManager{ inputMgr }
        , m_themeManager{ themeMgr }
        , m_animManager{ animMgr }
    {}

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();

        return m_taskbar.Initialize(
            m_themeManager,
            m_inputManager.GetMouseManager(),
            m_windowManager,
            m_animManager);
    }

    void Shutdown() noexcept override
    {
        m_taskbar.Shutdown();
    }

    void Update(float deltaTime) noexcept override
    {
        m_taskbar.ProcessInput();
        m_taskbar.Update(deltaTime);
    }

    void Render(EngineContext& ctx) noexcept override
    {
        auto* renderer = ctx.GetRenderer();
        if (!renderer) { return; }

        m_taskbar.Render(*renderer);
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_taskbar.Resize(width, height);
    }

    [[nodiscard]] Taskbar::Taskbar& GetTaskbar() noexcept { return m_taskbar; }

private:
    Taskbar::Taskbar              m_taskbar;
    WindowManager::WindowManager& m_windowManager;
    Input::InputManager&          m_inputManager;
    Theme::ThemeManager&          m_themeManager;
    Animation::AnimationManager&  m_animManager;
    float                         m_viewportWidth{ 0.0f };
    float                         m_viewportHeight{ 0.0f };
};

// ── StartMenuSystem ───────────────────────────────────────────────────────

/**
 * @brief  System wrapper for StartMenu::StartMenuController.
 *
 * Renders the start menu on top of everything.
 * Manages open/close animations and input routing.
 */
class StartMenuSystem final : public System {
public:
    explicit StartMenuSystem(
        Theme::ThemeManager&         themeMgr,
        Input::InputManager&         inputMgr,
        Animation::AnimationManager& animMgr,
        Apps::ApplicationRegistry&   appRegistry) noexcept
        : m_themeManager{ themeMgr }
        , m_inputManager{ inputMgr }
        , m_animManager{ animMgr }
        , m_appRegistry{ appRegistry }
    {}

    void SetTaskbar(Taskbar::Taskbar& taskbar) noexcept
    {
        m_pTaskbar = &taskbar;
    }

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();

        if (!m_controller.Initialize(
                m_themeManager,
                m_inputManager.GetMouseManager(),
                m_animManager,
                m_appRegistry))
        {
            return false;
        }

        // Wire StartMenuController to Taskbar
        if (m_pTaskbar)
        {
            m_pTaskbar->SetStartMenuController(m_controller);
        }

        return true;
    }

    void Shutdown() noexcept override
    {
        m_controller.Shutdown();
    }

    void Update(float deltaTime) noexcept override
    {
        if (!m_controller.IsOpen() && !m_controller.WantsInput())
        {
            return;
        }

        m_controller.ProcessInput();
        m_controller.Update(deltaTime);
    }

    void Render(EngineContext& ctx) noexcept override
    {
        if (!m_controller.IsOpen() && !m_controller.WantsInput())
        {
            return;
        }

        auto* renderer = ctx.GetRenderer();
        if (!renderer) { return; }

        m_controller.Render(*renderer);
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_controller.Resize(width, height);
    }

    [[nodiscard]] StartMenu::StartMenuController& GetController() noexcept
    {
        return m_controller;
    }

private:
    StartMenu::StartMenuController    m_controller;
    Theme::ThemeManager&              m_themeManager;
    Input::InputManager&              m_inputManager;
    Animation::AnimationManager&      m_animManager;
    Apps::ApplicationRegistry&        m_appRegistry;
    Taskbar::Taskbar*                 m_pTaskbar{ nullptr };
    float                             m_viewportWidth{ 0.0f };
    float                             m_viewportHeight{ 0.0f };
};

// ── NotificationCenterSystem ─────────────────────────────────────────────

class NotificationCenterSystem final : public System {
public:
    explicit NotificationCenterSystem(
        Theme::ThemeManager& themeMgr,
        Input::InputManager& inputMgr) noexcept
        : m_themeManager{ themeMgr }
        , m_inputManager{ inputMgr }
    {}

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();

        return m_center.Initialize(
            m_themeManager,
            m_inputManager.GetMouseManager());
    }

    void Shutdown() noexcept override
    {
        m_center.Shutdown();
    }

    void Update(float deltaTime) noexcept override
    {
        if (m_center.IsOpen())
        {
            m_center.ProcessInput();
        }
        m_center.Update(deltaTime);
    }

    void Render(EngineContext& ctx) noexcept override
    {
        if (!m_center.IsOpen()) { return; }

        auto* renderer = ctx.GetRenderer();
        if (!renderer) { return; }

        m_center.Render(*renderer);
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        m_center.Resize(width, height);
    }

    Notifications::NotificationCenter& GetCenter() noexcept { return m_center; }

private:
    Notifications::NotificationCenter m_center;
    Theme::ThemeManager&              m_themeManager;
    Input::InputManager&              m_inputManager;
    float                             m_viewportWidth{ 0.0f };
    float                             m_viewportHeight{ 0.0f };
};

// ── PluginSystem ──────────────────────────────────────────────────────────

class PluginSystem final : public System {
public:
    PluginSystem() noexcept = default;

    bool Initialize(EngineContext& ctx) noexcept override
    {
        m_viewportWidth  = ctx.GetViewportWidth();
        m_viewportHeight = ctx.GetViewportHeight();

        // Create all SDK service adapters
        auto* themeMgr   = ctx.GetThemeManager();
        auto* inputMgr   = ctx.GetInputManager();

        if (m_pInternalNotifMgr)
        {
            m_pNotifSvc = std::make_unique<SDK::NotificationServiceAdapter>(
                *m_pInternalNotifMgr);
        }
        m_pConfigSvc = std::make_unique<SDK::ConfigServiceAdapter>();

        if (m_pInternalFileSvc)
        {
            m_pFileSvc = std::make_unique<SDK::FileServiceAdapter>(
                *m_pInternalFileSvc);
        }

        if (themeMgr)
        {
            m_pThemeSvc = std::make_unique<SDK::ThemeServiceAdapter>(*themeMgr);
        }

        if (inputMgr)
        {
            m_pInputSvc = std::make_unique<SDK::InputServiceAdapter>(*inputMgr);
        }

        // ResourceManager is optional — use fallback if not set
        auto& resMgrRef = m_pInternalResMgr
            ? *m_pInternalResMgr
            : m_fallbackResMgr;
        m_pResSvc = std::make_unique<SDK::ResourceServiceAdapter>(resMgrRef);

        if (m_pInternalWinMgr)
        {
            m_pWinSvc = std::make_unique<SDK::WindowServiceAdapter>(
                *m_pInternalWinMgr);
        }

        m_pMenuSvc  = std::make_unique<SDK::MenuServiceAdapter>();
        m_pDlgSvc   = std::make_unique<SDK::DialogServiceAdapter>();
        m_pEvBusAda = std::make_unique<SDK::EventBusAdapter>(m_eventBus);

        // Wire service adapters to PluginManager
        m_pluginManager.SetNotificationService(m_pNotifSvc.get());
        m_pluginManager.SetConfigService(m_pConfigSvc.get());
        m_pluginManager.SetFileService(m_pFileSvc.get());
        m_pluginManager.SetThemeService(m_pThemeSvc.get());
        m_pluginManager.SetInputService(m_pInputSvc.get());
        m_pluginManager.SetResourceService(m_pResSvc.get());
        m_pluginManager.SetWindowService(m_pWinSvc.get());
        m_pluginManager.SetMenuService(m_pMenuSvc.get());
        m_pluginManager.SetDialogService(m_pDlgSvc.get());
        m_pluginManager.SetEventBus(m_pEvBusAda.get());

        m_pluginManager.SetExtensionPointManager(m_extensionPointMgr);

        m_pluginManager.SetOnPluginLoaded(
            [this](std::wstring_view pluginName)
            {
                // Publish event when a plugin loads
                dragonos::sdk::Event ev;
                ev.type = dragonos::sdk::EventType::PluginLoaded;
                ev.sourceName = std::wstring{ pluginName };
                m_eventBus.Publish(ev);
            });

        m_pluginManager.SetOnPluginUnloaded(
            [this](std::wstring_view pluginName)
            {
                dragonos::sdk::Event ev;
                ev.type = dragonos::sdk::EventType::PluginUnloaded;
                ev.sourceName = std::wstring{ pluginName };
                m_eventBus.Publish(ev);
            });

        return m_pluginManager.Initialize(ctx);
    }

    void SetPointers(
        Notifications::NotificationManager* notifMgr,
        FileSystem::FileSystemService*      fileSvc,
        Resources::ResourceManager*         resMgr,
        WindowManager::WindowManager*       winMgr) noexcept
    {
        m_pInternalNotifMgr = notifMgr;
        m_pInternalFileSvc  = fileSvc;
        m_pInternalResMgr   = resMgr;
        m_pInternalWinMgr   = winMgr;
    }

    void Shutdown() noexcept override
    {
        m_pluginManager.Shutdown();
        m_eventBus.ProcessAsyncEvents();

        m_pNotifSvc.reset();
        m_pConfigSvc.reset();
        m_pFileSvc.reset();
        m_pThemeSvc.reset();
        m_pInputSvc.reset();
        m_pResSvc.reset();
        m_pWinSvc.reset();
        m_pMenuSvc.reset();
        m_pDlgSvc.reset();
        m_pEvBusAda.reset();
    }

    void Update(float deltaTime) noexcept override
    {
        m_eventBus.ProcessAsyncEvents();
        m_pluginManager.Update(deltaTime);
    }

    void Render(EngineContext& /*ctx*/) noexcept override
    {
    }

    void Resize(float width, float height) noexcept override
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
    }

    Plugins::PluginManager& GetPluginManager() noexcept { return m_pluginManager; }
    Events::EventBus&       GetEventBus()       noexcept { return m_eventBus; }
    ExtensionPoints::ExtensionPointManager& GetExtensionPointMgr() noexcept
    {
        return m_extensionPointMgr;
    }

private:
    // Core
    Plugins::PluginManager              m_pluginManager;
    Events::EventBus                    m_eventBus;
    ExtensionPoints::ExtensionPointManager m_extensionPointMgr;

    // SDK service adapters
    std::unique_ptr<SDK::NotificationServiceAdapter> m_pNotifSvc;
    std::unique_ptr<SDK::ConfigServiceAdapter>       m_pConfigSvc;
    std::unique_ptr<SDK::FileServiceAdapter>         m_pFileSvc;
    std::unique_ptr<SDK::ThemeServiceAdapter>        m_pThemeSvc;
    std::unique_ptr<SDK::InputServiceAdapter>        m_pInputSvc;
    std::unique_ptr<SDK::ResourceServiceAdapter>     m_pResSvc;
    std::unique_ptr<SDK::WindowServiceAdapter>       m_pWinSvc;
    std::unique_ptr<SDK::MenuServiceAdapter>         m_pMenuSvc;
    std::unique_ptr<SDK::DialogServiceAdapter>       m_pDlgSvc;
    std::unique_ptr<SDK::EventBusAdapter>            m_pEvBusAda;

    // Internal system pointers (set externally before Initialize)
    Notifications::NotificationManager* m_pInternalNotifMgr{ nullptr };
    FileSystem::FileSystemService*      m_pInternalFileSvc{ nullptr };
    Resources::ResourceManager*         m_pInternalResMgr{ nullptr };
    WindowManager::WindowManager*       m_pInternalWinMgr{ nullptr };

    Resources::ResourceManager          m_fallbackResMgr;

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
};

} // anonymous namespace
} // namespace DragonOS::Engine

// ============================================================================
//  Engine implementation
// ============================================================================

namespace DragonOS::Engine {

// ── Construction / Destruction ───────────────────────────────────────────

Engine::Engine() noexcept
    : m_pContext{ std::make_unique<EngineContext>() }
    , m_pSystemManager{ std::make_unique<SystemManager>() }
{
}

Engine::~Engine() noexcept
{
    Shutdown();
}

// ── Lifecycle ────────────────────────────────────────────────────────────

bool Engine::Initialize(
    Graphics::Renderer&       renderer,
    Desktop::DesktopManager&  desktopManager,
    WindowManager::WindowManager& windowManager) noexcept
{
    if (m_initialized) { return true; }

    // ── Populate context ─────────────────────────────────────────────────
    m_pContext->SetRenderer(renderer);

    // ── Register systems (order: theme → anim → background → input → windows) ──
    auto* themeMgr = m_pSystemManager->Register<Theme::ThemeManager>();
    m_pContext->SetThemeManager(*themeMgr);

    auto* animMgr = m_pSystemManager->Register<Animation::AnimationManager>();

    m_pSystemManager->Register<DesktopSystem>(desktopManager);
    auto* inputMgr = m_pSystemManager->Register<Input::InputManager>();
    m_pContext->SetInputManager(*inputMgr);

    m_pSystemManager->Register<WindowManagerSystem>(windowManager);

    // ── Wire InputManager + AnimationManager to WindowManager ─────────────
    windowManager.SetMouseManager(inputMgr->GetMouseManager());
    windowManager.SetAnimationManager(*animMgr);

    // ── Register FileSystemService (data store, not a visual layer) ──────
    auto* fsService = m_pSystemManager->Register<FileSystem::FileSystemService>();

    // ── Register ExplorerSystem (renders inside window client areas) ────
    auto* explorerSys = m_pSystemManager->Register<Explorer::ExplorerSystem>(
        windowManager, *themeMgr, *animMgr, *fsService);
    explorerSys->SetMouseManager(inputMgr->GetMouseManager());

    // ── Register TaskbarSystem (renders above windows, below debug) ──────
    auto* taskbarSys = m_pSystemManager->Register<TaskbarSystem>(
        windowManager, *inputMgr, *themeMgr, *animMgr);

    // ── Register ApplicationRegistry (shared data store, not a visual layer) ──
    auto* appRegistry = m_pSystemManager->Register<Apps::ApplicationRegistry>();

    // ── Register ApplicationManager (runtime app tracking) ───────────────
    auto* appMgr = m_pSystemManager->Register<AppRuntime::ApplicationManager>();

    // ── Register ProcessManager (process tracking) ───────────────────────
    auto* procMgr = m_pSystemManager->Register<Process::ProcessManager>();

    // ── Register NotificationManager (non-visual notification queue) ─────
    auto* notifMgr = m_pSystemManager->Register<Notifications::NotificationManager>();

    // ── Register SearchService (non-visual search engine) ────────────────
    auto* searchService = m_pSystemManager->Register<Search::SearchService>();
    searchService->SetApplicationRegistry(*appRegistry);
    searchService->SetFileSystemService(*fsService);

    // ── Register ServiceManager (background service controller) ──────────
    auto* svcMgr = m_pSystemManager->Register<Services::ServiceManager>();

    // ── Register SessionManager (session persistence) ────────────────────
    auto* sessionMgr = m_pSystemManager->Register<Session::SessionManager>();
    sessionMgr->SetWindowManager(windowManager);
    sessionMgr->SetThemeManager(*themeMgr);
    sessionMgr->SetDesktopManager(desktopManager);
    sessionMgr->SetFileSystemService(*fsService);

    // ── Register NotificationCenterSystem (UI panel) ─────────────────────
    auto* notifCenterSys = m_pSystemManager->Register<NotificationCenterSystem>(
        *themeMgr, *inputMgr);
    notifCenterSys->GetCenter().SetNotificationManager(*notifMgr);

    // ── Register TerminalSystem ──────────────────────────────────────────
    auto* terminalSys = m_pSystemManager->Register<Terminal::TerminalSystem>(
        windowManager, *themeMgr, *animMgr, *fsService, *inputMgr);
    terminalSys->SetMouseManager(inputMgr->GetMouseManager());

    // ── Register StartMenuSystem (renders above taskbar, below debug) ────
    auto* startMenuSys = m_pSystemManager->Register<StartMenuSystem>(
        *themeMgr, *inputMgr, *animMgr, *appRegistry);

    // Wiring: StartMenuController needs Taskbar reference
    startMenuSys->SetTaskbar(taskbarSys->GetTaskbar());

    // Wire NotificationManager + ServiceManager to Taskbar
    auto& taskbar = taskbarSys->GetTaskbar();
    taskbar.SetNotificationManager(*notifMgr);
    taskbar.SetServiceManager(*svcMgr);

    // Wire notification toggle callback (tray notifications icon)
    taskbar.SetToggleNotificationCallback([notifCenterSys]()
    {
        notifCenterSys->GetCenter().Toggle();
    });

    // Wire search toggle callback (search button on taskbar)
    taskbar.SetToggleSearchCallback([searchService]()
    {
        // Future: open search overlay panel
    });

    // Wire NotificationManager to StartMenuController
    startMenuSys->GetController().SetNotificationManager(*notifMgr);

    // ── Register DevToolsManager (system monitor, debug console, dashboard) ──
    auto* devToolsSys = m_pSystemManager->Register<Diagnostics::DevToolsManager>(
        windowManager, *themeMgr, *animMgr, *inputMgr,
        *procMgr, *appMgr, *notifMgr);
    devToolsSys->SetMouseManager(inputMgr->GetMouseManager());

    // Wiring: StartMenu launch callback → app launcher
    startMenuSys->GetController().SetLaunchAppCallback(
        [explorerSys, terminalSys, devToolsSys, appMgr, procMgr](const Apps::AppInfo* appInfo)
        {
            if (!appInfo) { return; }
            if (appInfo->name == L"Explorer" || appInfo->name == L"explorer")
            {
                explorerSys->OpenExplorer();
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
            else if (appInfo->name == L"Terminal" || appInfo->name == L"terminal")
            {
                terminalSys->OpenTerminal();
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
            else if (appInfo->name == L"Settings" || appInfo->name == L"Settings")
            {
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
            else if (appInfo->name == L"System Monitor" || appInfo->name == L"SystemMonitor")
            {
                devToolsSys->OpenSystemMonitor();
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
            else if (appInfo->name == L"Debug Console" || appInfo->name == L"DebugConsole")
            {
                devToolsSys->OpenDebugConsole();
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
            else if (appInfo->name == L"Diagnostic Dashboard" || appInfo->name == L"DiagnosticDashboard")
            {
                devToolsSys->OpenDashboard();
                appMgr->RegisterApplication(
                    appInfo->id, appInfo->name, appInfo->displayName);
                procMgr->SpawnProcess(appInfo->name + L".exe");
            }
        });

    // ── Register debug overlay ──────────────────────────────────────────
    auto* debugOverlay = m_pSystemManager->Register<Input::DebugOverlay>();
    debugOverlay->SetInputManager(*inputMgr);
    debugOverlay->SetWindowManager(windowManager);
    debugOverlay->SetApplicationManager(*appMgr);
    debugOverlay->SetProcessManager(*procMgr);

    // ── Register PluginSystem (orchestrates plugin lifecycle) ────────────
    auto* pluginSys = m_pSystemManager->Register<PluginSystem>();

    // Set internal pointers before Initialize
    pluginSys->SetPointers(notifMgr, fsService, nullptr, &windowManager);

    // ── Register SecuritySystem (login screen, user management, permissions) ──
    auto* securitySys = m_pSystemManager->Register<Security::SecuritySystem>();
    securitySys->SetThemeManager(*themeMgr);
    securitySys->SetInputManager(*inputMgr);
    securitySys->SetMouseManager(inputMgr->GetMouseManager());

    // ── Initialise all systems ───────────────────────────────────────────
    m_pSystemManager->InitializeAll(*m_pContext);

    // ── Post-initialization wiring ───────────────────────────────────────
    // Wire PluginManager -> PermissionManager for plugin security validation
    pluginSys->GetPluginManager().SetPermissionManager(
        &securitySys->GetPermissionManager());

    m_initialized = true;
    return true;
}

void Engine::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_pSystemManager->ShutdownAll();
    m_initialized = false;
}

// ── Per-frame ────────────────────────────────────────────────────────────

void Engine::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    m_pSystemManager->UpdateAll(deltaTime);
}

void Engine::Render() noexcept
{
    if (!m_initialized) { return; }

    auto* renderer = m_pContext->GetRenderer();
    if (!renderer) { return; }

    renderer->BeginFrame();
    m_pSystemManager->RenderAll(*m_pContext);
    renderer->EndFrame();
}

void Engine::Resize(float width, float height) noexcept
{
    if (!m_initialized) { return; }

    m_pContext->SetViewport(width, height);
    m_pSystemManager->ResizeAll(width, height);
}

} // namespace DragonOS::Engine
