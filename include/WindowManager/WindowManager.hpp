/**
 * @file    WindowManager.hpp
 * @brief   Top-level controller for the DragonOS window system.
 *
 * WindowManager manages the lifetime, Z-order, focus, and rendering
 * of every DragonWindow.  It is owned by DesktopManager and called
 * during the per-frame Render / Update cycle.
 */

#pragma once

#include <Input/HitTest.hpp>
#include <Input/MouseButtons.hpp>
#include <Input/UIEvents.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/WindowCollection.hpp>
#include <WindowManager/SnapLayout.hpp>

#include <string_view>
#include <vector>

namespace DragonOS::Animation { class AnimationManager; }
namespace DragonOS::Graphics  { class Renderer; }
namespace DragonOS::Input    { class MouseManager; }

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
 *   - Per-frame hit testing and mouse-event routing.
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

    /// @brief  Advance animation state and process input for all windows.
    void Update(float deltaTime) noexcept;

    /// @brief  Notify windows that the viewport has changed.
    void Resize(float width, float height) noexcept;

    // ── Input integration ─────────────────────────────────────────────────

    /// @brief  Set the active MouseManager for hit-testing (called once).
    void SetMouseManager(Input::MouseManager& mgr) noexcept { m_pMouse = &mgr; }

    /// @brief  Run hit-test logic for the current frame.
    void ProcessInput() noexcept;

    // ── Event access ──────────────────────────────────────────────────────

    /// @brief  Read-only access to the current frame's UI events.
    [[nodiscard]] const std::vector<Input::UIEvent>& GetEvents() const noexcept
    {
        return m_events;
    }

    /// @brief  Access the hovered window (may be nullptr).
    [[nodiscard]] DragonWindow* GetHoveredWindow() const noexcept
    {
        return m_pHovered;
    }

    /// @brief  Access the focused window (may be nullptr).
    [[nodiscard]] DragonWindow* GetFocusedWindow() const noexcept
    {
        return m_pFocused;
    }

    /// @brief  The region under the cursor on the hovered window.
    [[nodiscard]] Input::HitTestRegion GetHitRegion() const noexcept
    {
        return m_hitRegion;
    }

    // ── Window management ─────────────────────────────────────────────────

    /// @brief  Add a window (placed at the top of the Z-order).
    DragonWindow* AddWindow(std::unique_ptr<DragonWindow> window) noexcept;

    /// @brief  Remove a window from the system.
    bool RemoveWindow(DragonWindow* window) noexcept;

    /// @brief  Promote a window to the top of the Z-order.
    void BringToFront(DragonWindow* window) noexcept;

    /// @brief  Send a window to the bottom of the Z-order.
    void SendToBack(DragonWindow* window) noexcept;

    /// @brief  Find the first window matching a title.
    /// @note  Named FindWindowByTitle to avoid Windows macro #define FindWindow FindWindowW.
    [[nodiscard]] DragonWindow* FindWindowByTitle(std::wstring_view title) noexcept;

    /// @brief  Set the keyboard-focus window.
    void SetFocusedWindow(DragonWindow* window) noexcept;

    /// @brief  Toggle between normal and maximised state for a window.
    void ToggleMaximize(DragonWindow* window) noexcept;

    /// @brief  Set the AnimationManager for animated transitions.
    void SetAnimationManager(Animation::AnimationManager& mgr) noexcept
    {
        m_pAnimationManager = &mgr;
    }

    /// @brief  Access the underlying collection.
    [[nodiscard]] WindowCollection& GetCollection() noexcept { return m_collection; }

private:
    void PushEvent(Input::UIEvent ev) noexcept { m_events.push_back(ev); }

    // ── Drag / Resize helpers ─────────────────────────────────────────────

    [[nodiscard]] static bool IsResizeRegion(Input::HitTestRegion r) noexcept
    {
        return r >= Input::HitTestRegion::BorderLeft &&
               r <= Input::HitTestRegion::BorderBottomRight;
    }

    void StartDrag(DragonWindow* window, float mouseX, float mouseY) noexcept;
    void UpdateDrag(float mouseX, float mouseY) noexcept;
    void EndDrag() noexcept;

    void StartResize(DragonWindow* window, Input::HitTestRegion edge,
                     float mouseX, float mouseY) noexcept;
    void UpdateResize(float mouseX, float mouseY) noexcept;
    void EndResize() noexcept;

    void HandleControlClick(
        DragonWindow* window, Input::HitTestRegion region) noexcept;
    void UpdateControlHover(DragonWindow* window, Input::HitTestRegion region) noexcept;
    void RenderSnapIndicator(Graphics::Renderer& renderer) noexcept;

    WindowCollection  m_collection;
    DragonWindow*     m_pFocused{ nullptr };
    DragonWindow*     m_pHovered{ nullptr };
    Input::MouseManager* m_pMouse{ nullptr };
    Animation::AnimationManager* m_pAnimationManager{ nullptr };
    Input::HitTestRegion m_hitRegion{ Input::HitTestRegion::None };
    bool              m_initialized{ false };

    // ── Viewport dimensions (set by Render/Resize) ─────────────────────────
    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };

    // ── Drag state ─────────────────────────────────────────────────────────
    bool          m_isDragging{ false };
    DragonWindow* m_pDragWindow{ nullptr };
    float         m_dragOffsetX{ 0.0f };
    float         m_dragOffsetY{ 0.0f };

    // ── Resize state ────────────────────────────────────────────────────────
    bool                m_isResizing{ false };
    DragonWindow*       m_pResizeWindow{ nullptr };
    Input::HitTestRegion m_resizeEdge{ Input::HitTestRegion::None };
    Input::Bounds       m_resizeAnchor{};
    Input::Point        m_resizeAnchorPoint{};

    // ── Snap state ─────────────────────────────────────────────────────────
    SnapRegion m_activeSnapRegion{ SnapRegion::None };

    // ── Per-frame event queue ──────────────────────────────────────────────
    std::vector<Input::UIEvent> m_events;
};

} // namespace DragonOS::WindowManager
