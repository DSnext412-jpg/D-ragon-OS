/**
 * @file    Application.cpp
 * @brief   Implementation of the Application class.
 */

#include <Core/Application.hpp>
#include <Engine/Engine.hpp>
#include <Engine/SystemManager.hpp>
#include <Input/InputManager.hpp>
#include <Window/Window.hpp>
#include <Config/Config.hpp>

#include <cstdlib>
#include <exception>

namespace DragonOS::Core {

Application::~Application() noexcept
{
    m_pEngine.reset();
    m_pWindow.reset();
}

// ============================================================================
//  Initialize
// ============================================================================

bool Application::Initialize(
    HINSTANCE hInstance,
    int       nCmdShow) noexcept
{
    try
    {
        // ── Create the main window ─────────────────────────────────────────
        m_pWindow = std::make_unique<DragonOS::Window::Window>(
            hInstance,
            std::wstring{ Config::WindowClassName },
            std::wstring{ Config::WindowTitle },
            Config::DefaultWindowWidth,
            Config::DefaultWindowHeight);

        // ── Create and wire the engine ──────────────────────────────────────
        m_pEngine = std::make_unique<DragonOS::Engine::Engine>();

        if (!m_pEngine->Initialize(
                m_pWindow->GetRenderer(),
                m_pWindow->GetDesktopManager(),
                m_pWindow->GetDesktopManager().GetWindowManager()))
        {
            m_pWindow.reset();
            m_pEngine.reset();
            return false;
        }

        // ── Wire the input manager to the window ────────────────────────────
        auto* inputMgr = m_pEngine->GetSystemManager()
                            .Find<DragonOS::Input::InputManager>();
        if (inputMgr)
        {
            m_pWindow->SetInputManager(*inputMgr);
        }

        m_pWindow->SetEngine(*m_pEngine);

        m_pWindow->Show(nCmdShow);
        m_pWindow->Update();

        return true;
    }
    catch (const std::exception& ex)
    {
        ::MessageBoxA(nullptr, ex.what(),
                      "DragonOS – Initialization Error",
                      MB_OK | MB_ICONERROR);
        m_pWindow.reset();
        return false;
    }
}

// ============================================================================
//  Run — message loop
// ============================================================================

int Application::Run() noexcept
{
    if (!m_pWindow)
    {
        return EXIT_FAILURE;
    }

    MSG msg{};
    BOOL result = TRUE;

    while ((result = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
    {
        if (result == -1)
        {
            // Fatal error — return immediately.
            return EXIT_FAILURE;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);

        // Advance per-frame state for all engine systems.
        if (m_pEngine)
        {
            m_pEngine->Update(0.0f);
        }
    }

    // msg.wParam contains the exit code from PostQuitMessage.
    return static_cast<int>(msg.wParam);
}

// ============================================================================
//  Shutdown
// ============================================================================

void Application::Shutdown() noexcept
{
    // Shut down the engine before the window so systems can still
    // access window-owned resources during teardown (if needed).
    if (m_pEngine)
    {
        m_pEngine->Shutdown();
        m_pEngine.reset();
    }

    if (m_pWindow)
    {
        m_pWindow->Close();
        m_pWindow.reset();
    }
}

} // namespace DragonOS::Core
