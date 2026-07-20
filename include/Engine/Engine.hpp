/**
 * @file    Engine.hpp
 * @brief   Top-level coordinator for all runtime systems.
 *
 * The Engine is the central hub that owns the SystemManager and
 * EngineContext.  It drives the per-frame lifecycle (Update, Render)
 * and provides the single entry point for subsystem coordination.
 *
 * Ownership chain:
 *   Application → Engine → SystemManager → System (Renderer, Desktop, …)
 */

#pragma once

#include <memory>

namespace DragonOS::Engine {

class EngineContext;
class SystemManager;
class System;

} // namespace DragonOS::Engine

// ── Forward declarations of existing subsystems ──────────────────────────

namespace DragonOS::Graphics      { class Renderer; }
namespace DragonOS::Desktop       { class DesktopManager; }
namespace DragonOS::WindowManager { class WindowManager; }
namespace DragonOS::Theme         { class ThemeManager; }

namespace DragonOS::Engine {

/**
 * @brief  Coordinates all runtime subsystems of DragonOS.
 *
 * Responsibilities:
 *   - Initialise and own the SystemManager and EngineContext.
 *   - Register concrete System wrappers for every subsystem.
 *   - Drive per-frame Update / Render / Resize.
 *   - Provide a single Shutdown path for all systems.
 */
class Engine final {
public:
    Engine() noexcept;
    ~Engine() noexcept;

    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(Engine&&)      = delete;

    // ── Lifecycle ───────────────────────────────────────────────────────

    /**
     * @brief  Initialise the engine and register all current subsystems.
     *
     * @param renderer        Active Graphics::Renderer.
     * @param desktopManager  Active Desktop::DesktopManager.
     * @param windowManager   Active WindowManager::WindowManager.
     *
     * @return true on success.
     */
    [[nodiscard]] bool Initialize(
        Graphics::Renderer&       renderer,
        Desktop::DesktopManager&  desktopManager,
        WindowManager::WindowManager& windowManager) noexcept;

    /// @brief  Shut down every registered system.
    void Shutdown() noexcept;

    // ── Per-frame ───────────────────────────────────────────────────────

    /// @brief  Advance non-rendering state for all systems.
    void Update(float deltaTime) noexcept;

    /**
     * @brief  Render the entire scene.
     *
     * Calls BeginFrame on the renderer, then iterates SystemManager
     * to render each system, then calls EndFrame.
     */
    void Render() noexcept;

    /// @brief  Propagate a viewport resize to all systems.
    void Resize(float width, float height) noexcept;

    // ── Accessors ───────────────────────────────────────────────────────

    [[nodiscard]] EngineContext&   GetContext()       noexcept { return *m_pContext; }
    [[nodiscard]] SystemManager&   GetSystemManager() noexcept { return *m_pSystemManager; }

private:
    std::unique_ptr<EngineContext>   m_pContext;
    std::unique_ptr<SystemManager>   m_pSystemManager;
    bool                             m_initialized{ false };
};

} // namespace DragonOS::Engine
