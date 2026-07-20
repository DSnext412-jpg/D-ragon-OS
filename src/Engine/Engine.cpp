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
#include <Apps/ApplicationRegistry.hpp>
#include <Desktop/DesktopManager.hpp>
#include <Explorer/ExplorerSystem.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/DebugOverlay.hpp>
#include <Input/InputManager.hpp>
#include <StartMenu/StartMenuController.hpp>
#include <Taskbar/Taskbar.hpp>
#include <Theme/ThemeManager.hpp>
#include <WindowManager/WindowManager.hpp>

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

    // ── Register StartMenuSystem (renders above taskbar, below debug) ────
    auto* startMenuSys = m_pSystemManager->Register<StartMenuSystem>(
        *themeMgr, *inputMgr, *animMgr, *appRegistry);

    // Wiring: StartMenuController needs Taskbar reference
    startMenuSys->SetTaskbar(taskbarSys->GetTaskbar());

    // Wiring: StartMenu launch callback → ExplorerSystem
    startMenuSys->GetController().SetLaunchAppCallback(
        [explorerSys](const Apps::AppInfo* appInfo)
        {
            if (!appInfo) { return; }
            if (appInfo->name == L"Explorer" || appInfo->name == L"explorer")
            {
                explorerSys->OpenExplorer();
            }
        });

    // ── Register debug overlay (last, so it renders on top) ──────────────
    auto* debugOverlay = m_pSystemManager->Register<Input::DebugOverlay>();
    debugOverlay->SetInputManager(*inputMgr);
    debugOverlay->SetWindowManager(windowManager);

    // ── Initialise all systems ───────────────────────────────────────────
    m_pSystemManager->InitializeAll(*m_pContext);

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
