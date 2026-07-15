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

#include <Graphics/Renderer.hpp>
#include <Desktop/DesktopManager.hpp>
#include <Input/InputManager.hpp>
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

    // ── Register systems (order: theme → background → input → windows) ──
    auto* themeMgr = m_pSystemManager->Register<Theme::ThemeManager>();
    m_pContext->SetThemeManager(*themeMgr);

    m_pSystemManager->Register<DesktopSystem>(desktopManager);
    m_pSystemManager->Register<Input::InputManager>();
    m_pSystemManager->Register<WindowManagerSystem>(windowManager);

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
