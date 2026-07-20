/**
 * @file    SystemManager.cpp
 * @brief   Implementation of the SystemManager class.
 */

#include <Engine/SystemManager.hpp>
#include <Engine/System.hpp>
#include <Engine/EngineContext.hpp>

#include <algorithm>

namespace DragonOS::Engine {

SystemManager::~SystemManager() noexcept
{
    // Destructor defined out-of-line so unique_ptr<System> deleter
    // can see the complete System type.
}

// ============================================================================
//  Registration
// ============================================================================

bool SystemManager::Remove(System* system) noexcept
{
    if (!system) { return false; }

    const auto it = std::find_if(
        m_systems.begin(), m_systems.end(),
        [system](const auto& ptr) { return ptr.get() == system; });

    if (it == m_systems.end()) { return false; }

    (*it)->Shutdown();
    m_systems.erase(it);
    return true;
}

// ============================================================================
//  Batch lifecycle
// ============================================================================

void SystemManager::InitializeAll(EngineContext& ctx) noexcept
{
    for (auto& s : m_systems)
    {
        if (s) { [[maybe_unused]] const bool ok = s->Initialize(ctx); }
    }
}

void SystemManager::ShutdownAll() noexcept
{
    // Destroy in reverse order (foreground → background).
    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it)
    {
        if (*it) { (*it)->Shutdown(); }
    }
    m_systems.clear();
}

void SystemManager::UpdateAll(float deltaTime) noexcept
{
    for (auto& s : m_systems)
    {
        if (s) { s->Update(deltaTime); }
    }
}

void SystemManager::RenderAll(EngineContext& ctx) noexcept
{
    for (auto& s : m_systems)
    {
        if (s) { s->Render(ctx); }
    }
}

void SystemManager::ResizeAll(float width, float height) noexcept
{
    for (auto& s : m_systems)
    {
        if (s) { s->Resize(width, height); }
    }
}

} // namespace DragonOS::Engine
