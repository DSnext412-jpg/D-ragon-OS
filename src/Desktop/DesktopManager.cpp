/**
 * @file    DesktopManager.cpp
 * @brief   Implementation of the DesktopManager class.
 */

#include <Desktop/DesktopManager.hpp>
#include <Desktop/DesktopScene.hpp>
#include <Graphics/Renderer.hpp>
#include <WindowManager/WindowManager.hpp>

namespace DragonOS::Desktop {

// ============================================================================
//  Construction / Destruction
// ============================================================================

DesktopManager::~DesktopManager() noexcept
{
    Shutdown();
}

// ============================================================================
//  Lifecycle
// ============================================================================

bool DesktopManager::Initialize() noexcept
{
    if (m_initialized) { return true; }

    m_pScene = std::make_unique<DesktopScene>();

    m_pWindowManager = std::make_unique<WindowManager::WindowManager>();

    if (!m_pWindowManager->Initialize())
    {
        m_pWindowManager.reset();
        m_pScene.reset();
        return false;
    }

    m_initialized = true;
    return true;
}

void DesktopManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_pWindowManager.reset();
    m_pScene.reset();
    m_initialized = false;
}

// ============================================================================
//  Per-frame
// ============================================================================

void DesktopManager::Render(
    Graphics::Renderer& renderer,
    float               width,
    float               height) noexcept
{
    if (!m_initialized) { return; }

    // 1. Background (wallpaper, desktop icons in future).
    if (m_pScene)
    {
        m_pScene->Render(renderer, width, height);
    }

    // 2. Application windows on top of the background.
    if (m_pWindowManager)
    {
        m_pWindowManager->Render(renderer, width, height);
    }
}

void DesktopManager::Resize(float width, float height) noexcept
{
    if (!m_initialized) { return; }

    if (m_pScene)       { m_pScene->Resize(width, height); }
    if (m_pWindowManager) { m_pWindowManager->Resize(width, height); }
}

void DesktopManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    if (m_pScene)       { m_pScene->Update(deltaTime); }
    if (m_pWindowManager) { m_pWindowManager->Update(deltaTime); }
}

} // namespace DragonOS::Desktop
