/**
 * @file    DesktopManager.hpp
 * @brief   Top-level desktop environment controller.
 *
 * DesktopManager is the bridge between the Window / Renderer layer
 * and the Desktop scene graph.  It owns the DesktopScene (background
 * wallpaper) and the WindowManager (application windows) and
 * delegates Render, Resize, and Update calls to both.
 *
 * Ownership chain:
 *   Application → Window → DesktopManager → DesktopScene → Desktop → Wallpaper
 *                                        → WindowManager → WindowCollection → DragonWindow
 */

#pragma once

#include <WindowManager/WindowManager.hpp>
#include <Desktop/DesktopScene.hpp>

#include <memory>

namespace DragonOS::Graphics { class Renderer; }

namespace DragonOS::Desktop {

/**
 * @brief  Controls the desktop environment lifecycle.
 *
 * Responsibilities:
 *   - Initialise the desktop scene graph.
 *   - Forward Render to the scene.
 *   - Forward Resize to the scene.
 *   - Drive per-frame Update for animations.
 *   - Clean shutdown on exit.
 */
class DesktopManager final {
public:
    DesktopManager() noexcept = default;
    ~DesktopManager() noexcept;

    DesktopManager(const DesktopManager&)            = delete;
    DesktopManager& operator=(const DesktopManager&) = delete;
    DesktopManager(DesktopManager&&)                 = delete;
    DesktopManager& operator=(DesktopManager&&)      = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// @brief  Create the desktop scene graph.
    /// @return true on success.
    [[nodiscard]] bool Initialize() noexcept;

    /// @brief  Tear down the scene graph and release resources.
    void Shutdown() noexcept;

    // ── Per-frame ─────────────────────────────────────────────────────────

    /**
     * @brief  Render the entire desktop scene.
     *
     * Must be called inside a Renderer BeginFrame / EndFrame pair.
     *
     * @param renderer  Target renderer.
     * @param width     Client-area width  in DIPs.
     * @param height    Client-area height in DIPs.
     */
    void Render(
        Graphics::Renderer& renderer,
        float               width,
        float               height) noexcept;

    /// @brief  Resize every scene object to match a new client area.
    void Resize(float width, float height) noexcept;

    /**
     * @brief  Advance animation state by @p deltaTime  seconds.
     *
     * Currently a no-op; reserved for future animations.
     */
    void Update(float deltaTime) noexcept;

    /// @brief  Access the scene root (for future object insertion).
    [[nodiscard]] DesktopScene& GetScene() noexcept { return *m_pScene; }

    /// @brief  Access the window manager.
    [[nodiscard]] WindowManager::WindowManager& GetWindowManager() noexcept
    {
        return *m_pWindowManager;
    }

private:
    std::unique_ptr<DesktopScene>              m_pScene;
    std::unique_ptr<WindowManager::WindowManager> m_pWindowManager;
    bool                                       m_initialized{ false };
};

} // namespace DragonOS::Desktop
