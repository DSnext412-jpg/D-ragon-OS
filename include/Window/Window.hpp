/**
 * @file    Window.hpp
 * @brief   Reusable, RAII-based native Win32 window.
 *
 * The Window class encapsulates every Win32 detail of window-class
 * registration, window creation, message dispatch, and cleanup.
 * Consumers interact only with a clean C++ interface.
 */

#pragma once

#include <Windows.h>
#include <string>
#include <string_view>

#include <Desktop/DesktopManager.hpp>
#include <Graphics/Renderer.hpp>

namespace DragonOS::Engine { class Engine; }
namespace DragonOS::Input  { class InputManager; }

namespace DragonOS::Window {

/**
 * @brief  RAII wrapper around a native Win32 window.
 *
 * Registers the window class upon construction, creates and shows the
 * window, and unregisters the class upon destruction.  The static
 * WndProc retrieves the Window instance and forwards all messages to
 * the HandleMessage() instance method.
 */
class Window final {
public:
    /**
     * @brief  Register the window class and create the native window.
     *
     * @param hInstance  Application instance handle.
     * @param className  Name for the Win32 window class.
     * @param title      Text displayed in the title bar.
     * @param width      Desired client-area width.
     * @param height     Desired client-area height.
     *
     * @throws std::runtime_error if registration or creation fails.
     */
    Window(
        HINSTANCE        hInstance,
        std::wstring     className,
        const std::wstring& title,
        int              width,
        int              height);

    /**
     * @brief  Destroy the native window and unregister the class.
     *
     * If the window was already destroyed (e.g. via Close()), the
     * operation is idempotent.
     */
    ~Window() noexcept;

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&)                 = delete;
    Window& operator=(Window&&)      = delete;

    /// @brief  Show the window.  @p nCmdShow  accepts SW_SHOW, SW_HIDE, etc.
    void Show(int nCmdShow) noexcept;

    /// @brief  Force an immediate repaint of the client area.
    void Update() noexcept;

    /// @brief  Programmatically destroy the native window.
    void Close() noexcept;

    /// @brief  Retrieve the underlying HWND (nullptr if destroyed).
    [[nodiscard]] HWND Handle() const noexcept;

    // ── Subsystem access ────────────────────────────────────────────────

    [[nodiscard]] Graphics::Renderer&         GetRenderer()       noexcept { return m_renderer; }
    [[nodiscard]] Desktop::DesktopManager&    GetDesktopManager() noexcept { return m_desktopManager; }
    void                                      SetEngine(Engine::Engine& engine) noexcept { m_pEngine = &engine; }
    void                                      SetInputManager(Input::InputManager& mgr) noexcept { m_pInputManager = &mgr; }

private:
    // ── Window procedure ───────────────────────────────────────────────

    /**
     * @brief  Static Win32 window procedure.
     *
     * On WM_NCCREATE, stores the Window pointer in the window's
     * GWLP_USERDATA slot.  All subsequent messages are forwarded to
     * HandleMessage() on the correct instance.
     */
    static LRESULT CALLBACK WndProc(
        HWND   hWnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam) noexcept;

    // ── Instance message handlers ──────────────────────────────────────

    /// @brief  Central message dispatcher.
    LRESULT HandleMessage(
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam) noexcept;

    /// @brief  WM_CLOSE  — initiate window destruction.
    void OnClose() noexcept;

    /// @brief  WM_DESTROY — post WM_QUIT to the message queue.
    void OnDestroy() noexcept;

    /// @brief  WM_PAINT   — validate the client area.
    void OnPaint() noexcept;

    /// @brief  WM_SIZE    — respond to size changes.
    void OnResize(LPARAM lParam) noexcept;

    // ── Data members ───────────────────────────────────────────────────

    HWND                       m_hWnd{ nullptr };
    HINSTANCE                  m_hInstance;
    std::wstring               m_className;
    int                        m_width{ 0 };
    int                        m_height{ 0 };
    Graphics::Renderer         m_renderer;
    Desktop::DesktopManager    m_desktopManager;
    Engine::Engine*            m_pEngine{ nullptr };
    Input::InputManager*       m_pInputManager{ nullptr };
};

} // namespace DragonOS::Window
