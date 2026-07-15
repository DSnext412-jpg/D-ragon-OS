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
#include <WindowManager/WindowStyle.hpp>
#include <WindowManager/WindowState.hpp>

#include <string>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme   { class ThemeManager; }

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

    // ── Focus ────────────────────────────────────────────────────────────

    /// @brief  Give this window keyboard / input focus.
    void Focus() noexcept;

    // ── Rendering ────────────────────────────────────────────────────────

    /// @brief  Draw the window (shadow, background, border, title text).
    void Render(Graphics::Renderer& renderer) noexcept;

    /**
     * @brief  Per-frame update (reserved for animations).
     * @param deltaTime  Seconds since the last frame.
     */
    void Update(float deltaTime) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

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

    // ── Future interaction stubs ─────────────────────────────────────────
    // int  HitTest(float px, float py) const;        // TODO
    // void BeginDrag(float px, float py);            // TODO
    // void BeginResize(int edge, float px, float py); // TODO
    // void SnapToEdge(int edge);                     // TODO
    // void DockTo(DragonWindow& target);             // TODO
    // void AnimateTo(float x, float y, float w, float h, float duration); // TODO

    // ── Theme integration ───────────────────────────────────────────────

    /// @brief  Attach a ThemeManager to drive all visual properties.
    void SetThemeManager(const Theme::ThemeManager& themeManager) noexcept;

private:
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

    const Theme::ThemeManager* m_pThemeManager{ nullptr };
};

} // namespace DragonOS::WindowManager
