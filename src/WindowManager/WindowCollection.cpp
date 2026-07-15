/**
 * @file    WindowCollection.cpp
 * @brief   Implementation of the WindowCollection class.
 */

#include <WindowManager/WindowCollection.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <Graphics/Renderer.hpp>

#include <algorithm>

namespace DragonOS::WindowManager {

WindowCollection::~WindowCollection() noexcept
{
    Clear();
}

void WindowCollection::Clear() noexcept
{
    m_windows.clear();
}

// ============================================================================
//  Window management
// ============================================================================

DragonWindow* WindowCollection::Add(
    std::unique_ptr<DragonWindow> window) noexcept
{
    if (!window) { return nullptr; }

    auto* raw = window.get();
    raw->SetZOrder(static_cast<int>(m_windows.size()));
    m_windows.push_back(std::move(window));
    return raw;
}

bool WindowCollection::Remove(DragonWindow* window) noexcept
{
    if (!window) { return false; }

    const auto it = std::find_if(
        m_windows.begin(), m_windows.end(),
        [window](const auto& ptr) { return ptr.get() == window; });

    if (it == m_windows.end()) { return false; }

    m_windows.erase(it);

    // Re-number Z-order.
    for (size_t i = 0; i < m_windows.size(); ++i)
    {
        m_windows[i]->SetZOrder(static_cast<int>(i));
    }

    return true;
}

void WindowCollection::BringToFront(DragonWindow* window) noexcept
{
    if (!window || m_windows.size() < 2) { return; }

    const auto it = std::find_if(
        m_windows.begin(), m_windows.end(),
        [window](const auto& ptr) { return ptr.get() == window; });

    if (it == m_windows.end()) { return; }

    // Rotate the matching element to the back of the vector
    // (iterated last → rendered last → top-most).
    auto ptr = std::move(*it);
    m_windows.erase(it);
    m_windows.push_back(std::move(ptr));

    // Re-number Z-order.
    for (size_t i = 0; i < m_windows.size(); ++i)
    {
        m_windows[i]->SetZOrder(static_cast<int>(i));
    }
}

DragonWindow* WindowCollection::Find(std::wstring_view title) noexcept
{
    for (auto& w : m_windows)
    {
        if (w->GetTitle() == title) { return w.get(); }
    }
    return nullptr;
}

// ============================================================================
//  Batch operations
// ============================================================================

void WindowCollection::Render(Graphics::Renderer& renderer) noexcept
{
    for (auto& w : m_windows)
    {
        w->Render(renderer);
    }
}

void WindowCollection::Update(float deltaTime) noexcept
{
    for (auto& w : m_windows)
    {
        w->Update(deltaTime);
    }
}

} // namespace DragonOS::WindowManager
