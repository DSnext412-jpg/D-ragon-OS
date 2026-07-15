/**
 * @file    WindowManager.hpp
 * @brief   Top-level controller for the DragonOS window system.
 *
 * WindowManager manages the lifetime, Z-order, focus, and rendering
 * of every DragonWindow.  It is owned by DesktopManager and called
 * during the per-frame Render / Update cycle.
 */

#pragma once

#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/WindowCollection.hpp>

#include <string_view>

namespace DragonOS::Graphics { class Renderer; }

namespace DragonOS::WindowManager {

/**
 * @brief  Controls every DragonWindow in the environment.
 *
 * Responsibilities:
 *   - Initialise the window collection.
 *   - Add / remove / reorder / query windows.
 *   - Render all windows (back-to-front).
 *   - Drive per-frame Update for animations.
 *   - Track the focused window.
 */
class WindowManager final {
public:
    WindowManager() noexcept = default;
    ~WindowManager() noexcept;

    WindowManager(const WindowManager&)            = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&)                 = delete;
    WindowManager& operator=(WindowManager&&)      = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// @brief  Initialise the window manager and create demo content.
    /// @return true on success.
    [[nodiscard]] bool Initialize() noexcept;

    /// @brief  Release all windows.
    void Shutdown() noexcept;

    // ── Per-frame ─────────────────────────────────────────────────────────

    /// @brief  Render every window (back-to-front) into the given renderer.
    void Render(
        Graphics::Renderer& renderer,
        float               viewportWidth,
        float               viewportHeight) noexcept;

    /// @brief  Advance animation state for all windows.
    void Update(float deltaTime) noexcept;

    /// @brief  Notify windows that the viewport has changed.
    void Resize(float width, float height) noexcept;

    // ── Window management ─────────────────────────────────────────────────

    /// @brief  Add a window (placed at the top of the Z-order).
    DragonWindow* AddWindow(std::unique_ptr<DragonWindow> window) noexcept;

    /// @brief  Remove a window from the system.
    bool RemoveWindow(DragonWindow* window) noexcept;

    /// @brief  Promote a window to the top of the Z-order.
    void BringToFront(DragonWindow* window) noexcept;

    /// @brief  Find the first window matching a title.
    /// @note  Named FindWindowByTitle to avoid Windows macro #define FindWindow FindWindowW.
    [[nodiscard]] DragonWindow* FindWindowByTitle(std::wstring_view title) noexcept;

    /// @brief  Set the keyboard-focus window.
    void SetFocusedWindow(DragonWindow* window) noexcept;

    /// @brief  Access the underlying collection.
    [[nodiscard]] WindowCollection& GetCollection() noexcept { return m_collection; }

private:
    WindowCollection m_collection;
    DragonWindow*    m_pFocused{ nullptr };
    bool             m_initialized{ false };
};

} // namespace DragonOS::WindowManager
