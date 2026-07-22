/**
 * @file    DragonWindow.hpp
 * @brief   A single window rendered by the DragonOS window system.
 *
 * DragonWindow is NOT a native Win32 window.  It is a lightweight
 * object that stores title, geometry, state, and style, and draws
 * itself using the Direct2D renderer.  All chrome (title bar, border,
 * shadow, close/min/max buttons) is rendered manually.
 */

#pragma once

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>
#include <WindowManager/WindowStyle.hpp>
#include <WindowManager/WindowState.hpp>

#include <cstdint>
#include <string>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme   { class ThemeManager; }
namespace DragonOS::Input   { struct UIEvent; }

namespace DragonOS::WindowManager {

/**
 * @brief  Represents a single window inside the DragonOS environment.
 *
 * Each DragonWindow owns its position, size, visual state, and style
 * flags.  It is rendered by calling Render() with a valid
 * Graphics::Renderer that is inside a BeginFrame / EndFrame pair.
 */
class DragonWindow final {
public:
    /**
     * @brief  Construct a window with a given title and geometry.
     *
     * @param title  Text displayed in the title bar.
     * @param x      Initial X position (DIPs).
     * @param y      Initial Y position (DIPs).
     * @param w      Client-area width  (DIPs).
     * @param h      Client-area height (DIPs).
     */
    DragonWindow(
        std::wstring  title,
        float         x,
        float         y,
        float         w,
        float         h) noexcept;

    ~DragonWindow() noexcept = default;

    DragonWindow(const DragonWindow&)            = delete;
    DragonWindow& operator=(const DragonWindow&) = delete;
    DragonWindow(DragonWindow&&)                 = default;
    DragonWindow& operator=(DragonWindow&&)      = default;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// @brief  Mark the window as open and visible.
    void Open() noexcept;

    /// @brief  Mark the window as closed (hidden).
    void Close() noexcept;

    /// @brief  Minimise the window.
    void Minimize() noexcept;

    /// @brief  Maximise the window to fill the viewport.
    void Maximize() noexcept;

    /// @brief  Restore from minimised or maximised state.
    void Restore() noexcept;

    // ── Geometry ─────────────────────────────────────────────────────────

    /// @brief  Move the window to an absolute position.
    void Move(float x, float y) noexcept;

    /// @brief  Resize the window.
    void Resize(float w, float h) noexcept;

    /// @brief  The bounding rectangle of the window in DIPs.
    [[nodiscard]] Input::Bounds GetBounds() const noexcept
    {
        return { m_x, m_y, m_width, m_height };
    }

    /**
     * @brief  Hit-test the window at the given client point.
     * @return The region under the point, or HitTestRegion::None.
     */
    [[nodiscard]] Input::HitTestRegion HitTest(float px, float py) const noexcept;

    // ── Focus ────────────────────────────────────────────────────────────

    /// @brief  Give this window keyboard / input focus.
    void Focus() noexcept;

    /// @brief  Remove keyboard / input focus.
    void FocusLost() noexcept;

    // ── Mouse state (updated by WindowManager each frame) ─────────────────

    void SetHovered(bool hovered) noexcept { m_hovered = hovered; }
    void SetPressed(bool pressed) noexcept { m_pressed = pressed; }

    [[nodiscard]] bool IsHovered() const noexcept { return m_hovered; }
    [[nodiscard]] bool IsPressed() const noexcept { return m_pressed; }
    [[nodiscard]] bool WasEnteredThisFrame()  const noexcept { return m_enteredThisFrame; }
    [[nodiscard]] bool WasExitedThisFrame()   const noexcept { return m_exitedThisFrame; }

    void SetEnteredThisFrame(bool v) noexcept { m_enteredThisFrame = v; }
    void SetExitedThisFrame(bool v)  noexcept { m_exitedThisFrame  = v; }

    // ── Rendering ────────────────────────────────────────────────────────

    /// @brief  Draw the window (shadow, background, border, title text).
    void Render(Graphics::Renderer& renderer) noexcept;

    /**
     * @brief  Per-frame update (reserved for animations).
     * @param deltaTime  Seconds since the last frame.
     */
    void Update(float deltaTime) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

    void SetTitle(std::wstring_view title) noexcept { m_title = title; }
    [[nodiscard]] const std::wstring& GetTitle()  const noexcept { return m_title; }
    [[nodiscard]] float GetX()      const noexcept { return m_x; }
    [[nodiscard]] float GetY()      const noexcept { return m_y; }
    [[nodiscard]] float GetWidth()  const noexcept { return m_width; }
    [[nodiscard]] float GetHeight() const noexcept { return m_height; }

    [[nodiscard]] bool        IsVisible() const noexcept { return m_visible; }
    [[nodiscard]] bool        IsFocused() const noexcept { return m_focused; }
    [[nodiscard]] int         GetZOrder() const noexcept { return m_zOrder; }
    [[nodiscard]] WindowState GetState()  const noexcept { return m_state; }
    [[nodiscard]] WindowStyle GetStyle()  const noexcept { return m_style; }

    void SetZOrder(int order) noexcept { m_zOrder = order; }
    void SetStyle(WindowStyle style) noexcept { m_style = style; }

    // ── Control button state (updated by WindowManager) ─────────────────────

    void SetActiveControlRegion(Input::HitTestRegion region) noexcept
    {
        m_activeControlRegion = region;
    }

    [[nodiscard]] Input::HitTestRegion GetActiveControlRegion() const noexcept
    {
        return m_activeControlRegion;
    }

    void SetPressedControlRegion(Input::HitTestRegion region) noexcept
    {
        m_pressedControlRegion = region;
    }

    [[nodiscard]] Input::HitTestRegion GetPressedControlRegion() const noexcept
    {
        return m_pressedControlRegion;
    }

    // ── Restore geometry (for maximize / minimize animations) ──────────────

    void SetRestoreGeometry(float x, float y, float w, float h) noexcept
    {
        m_restoreX = x; m_restoreY = y; m_restoreW = w; m_restoreH = h;
    }

    [[nodiscard]] float GetRestoreX() const noexcept { return m_restoreX; }
    [[nodiscard]] float GetRestoreY() const noexcept { return m_restoreY; }
    [[nodiscard]] float GetRestoreW() const noexcept { return m_restoreW; }
    [[nodiscard]] float GetRestoreH() const noexcept { return m_restoreH; }

    void SetMinWidth(float w) noexcept { m_minWidth = w; }
    void SetMinHeight(float h) noexcept { m_minHeight = h; }
    [[nodiscard]] float GetMinWidth() const noexcept { return m_minWidth; }
    [[nodiscard]] float GetMinHeight() const noexcept { return m_minHeight; }

    static constexpr float MinWindowWidth  = 200.0f;
    static constexpr float MinWindowHeight = 100.0f;

    // ── Theme integration ───────────────────────────────────────────────

    /// @brief  Attach a ThemeManager to drive all visual properties.
    void SetThemeManager(const Theme::ThemeManager& themeManager) noexcept;

    /// @brief  Unique identifier for this window.
    [[nodiscard]] uint64_t GetId() const noexcept { return m_id; }

private:
    static uint64_t NextId() noexcept
    {
        static uint64_t s_next = 1;
        return s_next++;
    }

    std::wstring  m_title;
    float         m_x;
    float         m_y;
    float         m_width;
    float         m_height;
    bool          m_visible{ true };
    bool          m_focused{ false };
    int           m_zOrder{ 0 };
    WindowState   m_state{ WindowState::Normal };
    WindowStyle   m_style{ WindowStyle::Resizable | WindowStyle::Closable | WindowStyle::Movable };
    uint64_t      m_id{ NextId() };
    float         m_minWidth{ MinWindowWidth };
    float         m_minHeight{ MinWindowHeight };

    // ── Mouse-interaction state (set by WindowManager each frame) ────────
    bool          m_hovered{ false };
    bool          m_pressed{ false };
    bool          m_enteredThisFrame{ false };
    bool          m_exitedThisFrame{ false };

    // ── Control button interaction state ─────────────────────────────────
    Input::HitTestRegion m_activeControlRegion{ Input::HitTestRegion::None };
    Input::HitTestRegion m_pressedControlRegion{ Input::HitTestRegion::None };

    // ── Restore geometry (before maximize / minimize) ────────────────────
    float m_restoreX{ 0.0f }, m_restoreY{ 0.0f };
    float m_restoreW{ 0.0f }, m_restoreH{ 0.0f };

    const Theme::ThemeManager* m_pThemeManager{ nullptr };
};

} // namespace DragonOS::WindowManager
