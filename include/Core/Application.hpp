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
#include <memory>

namespace DragonOS::Engine { class Engine; }
namespace DragonOS::Window  { class Window; }

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
    ~Application();

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
};

} // namespace DragonOS::Core
