/**
 * @file    Application.hpp
 * @brief   Application lifecycle controller.
 *
 * Core::Application is the top-level orchestrator.  It owns the main
 * window, drives initialisation, runs the Win32 message loop, and
 * coordinates clean shutdown.
 */

#pragma once

#include <Windows.h>
#include <cstdint>
#include <memory>

#include <Engine/Engine.hpp>
#include <Window/Window.hpp>

// ── Debug logging (compiled out in Release builds) ──────────────────────────

#ifdef _DEBUG
#define APP_LOG(msg)  ::OutputDebugStringW(L"[DragonOS] " msg L"\n")
#define APP_LOGF(...)                                                         \
    do {                                                                      \
        wchar_t _buf[512];                                                    \
        ::swprintf_s(_buf, __VA_ARGS__);                                      \
        ::OutputDebugStringW(L"[DragonOS] ");                                 \
        ::OutputDebugStringW(_buf);                                           \
        ::OutputDebugStringW(L"\n");                                          \
    } while (false)
#else
#define APP_LOG(msg)  ((void)0)
#define APP_LOGF(...) ((void)0)
#endif

namespace DragonOS::Core {

/**
 * @brief  Manages the application lifecycle.
 *
 * Responsibilities:
 *  - Initialise subsystems and create the main window.
 *  - Run the Win32 message loop (pump messages until WM_QUIT).
 *  - Shut down subsystems and destroy the main window.
 *
 * Usage:
 * @code
 *   Application app;
 *   if (app.Initialize(hInstance, nCmdShow))
 *   {
 *       int exitCode = app.Run();
 *       app.Shutdown();
 *       return exitCode;
 *   }
 *   return EXIT_FAILURE;
 * @endcode
 */
class Application final {
public:
    Application() = default;
    ~Application() noexcept;

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&)                 = delete;
    Application& operator=(Application&&)      = delete;

    /**
     * @brief  Create the main window and prepare for execution.
     *
     * @param hInstance  Handle to the application instance.
     * @param nCmdShow   How the window should be shown (SW_* values).
     *
     * @return true on success, false on failure (error dialog shown).
     */
    [[nodiscard]] bool Initialize(
        HINSTANCE hInstance,
        int       nCmdShow) noexcept;

    /**
     * @brief  Run the Win32 message loop until WM_QUIT is received.
     *
     * @return The exit code passed to PostQuitMessage, or EXIT_FAILURE
     *         if the message loop encounters a fatal error.
     */
    [[nodiscard]] int Run() noexcept;

    /**
     * @brief  Tear down the main window and release resources.
     *
     * Safe to call even if Initialize failed or was never called.
     */
    void Shutdown() noexcept;

private:
    std::unique_ptr<DragonOS::Window::Window> m_pWindow;
    std::unique_ptr<DragonOS::Engine::Engine> m_pEngine;

    // ── Loop state ──────────────────────────────────────────────────────
    bool   m_isRunning{ false };

    // ── High-resolution timer ───────────────────────────────────────────
    LARGE_INTEGER m_perfFreq{};
    LARGE_INTEGER m_prevTime{};
};

} // namespace DragonOS::Core
