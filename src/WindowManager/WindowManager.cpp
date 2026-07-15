/**
 * @file    WindowManager.cpp
 * @brief   Implementation of the WindowManager class.
 */

#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <Graphics/Renderer.hpp>

namespace DragonOS::WindowManager {

// ============================================================================
//  Lifecycle
// ============================================================================

bool WindowManager::Initialize() noexcept
{
    if (m_initialized) { return true; }

    // ── Demo window ───────────────────────────────────────────────────────
    auto explorer = std::make_unique<DragonWindow>(
        L"Explorer",
        120.0f,    // x
        100.0f,    // y
        800.0f,    // width
        500.0f);   // height

    explorer->Focus();
    m_pFocused = AddWindow(std::move(explorer));

    m_initialized = true;
    return true;
}

void WindowManager::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    m_pFocused = nullptr;
    m_collection.Clear();
    m_initialized = false;
}

// ============================================================================
//  Per-frame
// ============================================================================

void WindowManager::Render(
    Graphics::Renderer& renderer,
    float               /*viewportWidth*/,
    float               /*viewportHeight*/) noexcept
{
    if (!m_initialized) { return; }

    m_collection.Render(renderer);
}

void WindowManager::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    m_collection.Update(deltaTime);
}

void WindowManager::Resize(float /*width*/, float /*height*/) noexcept
{
    // Reserved — maximised windows will be resized to match here.
}

// ============================================================================
//  Window management
// ============================================================================

DragonWindow* WindowManager::AddWindow(
    std::unique_ptr<DragonWindow> window) noexcept
{
    return m_collection.Add(std::move(window));
}

bool WindowManager::RemoveWindow(DragonWindow* window) noexcept
{
    const bool removed = m_collection.Remove(window);

    if (removed && m_pFocused == window)
    {
        m_pFocused = nullptr;
    }

    return removed;
}

void WindowManager::BringToFront(DragonWindow* window) noexcept
{
    m_collection.BringToFront(window);
}

DragonWindow* WindowManager::FindWindowByTitle(
    std::wstring_view title) noexcept
{
    return m_collection.Find(title);
}

void WindowManager::SetFocusedWindow(DragonWindow* window) noexcept
{
    if (m_pFocused && m_pFocused != window)
    {
        // Lose focus on the old window (future event dispatch).
    }

    m_pFocused = window;

    if (m_pFocused)
    {
        m_pFocused->Focus();
        BringToFront(m_pFocused);
    }
}

} // namespace DragonOS::WindowManager
