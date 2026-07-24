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
        APP_LOG(L"Starting DragonOS initialisation");

        // ── Set up high-resolution timer ──────────────────────────────────
        ::QueryPerformanceFrequency(&m_perfFreq);
        ::QueryPerformanceCounter(&m_prevTime);

        // ── Create the main window ─────────────────────────────────────────
        m_pWindow = std::make_unique<DragonOS::Window::Window>(
            hInstance,
            std::wstring{ Config::WindowClassName },
            std::wstring{ Config::WindowTitle },
            Config::DefaultWindowWidth,
            Config::DefaultWindowHeight);

        APP_LOG(L"Window created successfully");

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

        APP_LOG(L"Engine initialised successfully");

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

        APP_LOG(L"Initialisation complete — window shown");
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
//  Run — real-time message / render loop
// ============================================================================

int Application::Run() noexcept
{
    if (!m_pWindow)
    {
        return EXIT_FAILURE;
    }

    m_isRunning = true;
    APP_LOG(L"Message loop started");

    MSG msg{};

    while (m_isRunning)
    {
        // ── 0. Clear previous frame's input buffers ─────────────────────
        auto* inputMgr = m_pEngine
            ? m_pEngine->GetSystemManager().Find<DragonOS::Input::InputManager>()
            : nullptr;
        if (inputMgr) { inputMgr->EndFrame(); }

        // ── 1. Process all pending Win32 messages (non-blocking) ─────────
        while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                m_isRunning = false;
                break;
            }

            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }

        if (!m_isRunning) { break; }

        // ── 2. Calculate delta time ──────────────────────────────────────
        LARGE_INTEGER currentTime{};
        ::QueryPerformanceCounter(&currentTime);

        const double delta = static_cast<double>(
            currentTime.QuadPart - m_prevTime.QuadPart) / m_perfFreq.QuadPart;
        m_prevTime = currentTime;

        // Clamp delta to ~50 ms to prevent spiral-of-death on first frame
        // or after a long stall (e.g. modal resize).
        const float deltaTime = (delta > 0.05) ? 0.016f
                                               : static_cast<float>(delta);

        // ── 3. Update engine systems (always) ─────────────────────────────
        if (m_pEngine)
        {
            m_pEngine->Update(deltaTime);
        }

        // ── 4. Render one frame (skip if minimised) ──────────────────────
        if (m_pEngine && !m_pWindow->IsMinimized())
        {
            m_pEngine->Render();
        }

        // ── 5. Yield CPU when appropriate ─────────────────────────────────
        if (m_pWindow->IsMinimized())
        {
            // Window is hidden — block until a message arrives.
            ::WaitMessage();
        }
        else if (!m_pWindow->IsActive())
        {
            // Visible but not foreground — brief yield keeps CPU low while
            // staying responsive for the next activation.
            ::Sleep(1);
        }
    }

    APP_LOG(L"Message loop exited");
    return static_cast<int>(msg.wParam);
}

// ============================================================================
//  Shutdown
// ============================================================================

void Application::Shutdown() noexcept
{
    APP_LOG(L"Shutting down DragonOS");

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

    APP_LOG(L"Shutdown complete");
}

} // namespace DragonOS::Core
