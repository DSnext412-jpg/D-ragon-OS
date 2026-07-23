#include <Settings/SettingsSystem.hpp>
#include <Settings/SettingsWindow.hpp>

#include <Animation/AnimationManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::Settings {

SettingsSystem::SettingsSystem(
    WindowManager::WindowManager& wndMgr,
    Theme::ThemeManager&          themeMgr) noexcept
    : m_windowManager{ wndMgr }
    , m_themeManager{ themeMgr }
{
}

bool SettingsSystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) { return true; }

    m_viewportWidth  = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_initialized = true;
    return true;
}

void SettingsSystem::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    for (auto& inst : m_instances)
    {
        if (inst.pWindow)
        {
            m_windowManager.RemoveWindow(inst.pWindow);
        }
    }
    m_instances.clear();
    m_initialized = false;
}

void SettingsSystem::Update(float /*deltaTime*/) noexcept
{
    if (!m_initialized) { return; }

    RemoveClosedWindows();

    for (auto& inst : m_instances)
    {
        if (inst.pContent && inst.pWindow && inst.pWindow->IsVisible())
        {
            inst.pContent->Update();
        }
    }
}

void SettingsSystem::Render(Engine::EngineContext& ctx) noexcept
{
    if (!m_initialized) { return; }

    auto* renderer = ctx.GetRenderer();
    if (!renderer) { return; }

    for (auto& inst : m_instances)
    {
        if (inst.pContent && inst.pWindow && inst.pWindow->IsVisible())
        {
            inst.pContent->Render(*renderer);
        }
    }
}

void SettingsSystem::Resize(float width, float height) noexcept
{
    m_viewportWidth  = width;
    m_viewportHeight = height;
}

void SettingsSystem::OpenSettings() noexcept
{
    if (!m_initialized) { return; }

    const float offsetX = 100.0f + (m_instances.size() % 5) * 30.0f;
    const float offsetY = 100.0f + (m_instances.size() % 5) * 30.0f;
    const float w = 800.0f;
    const float h = 550.0f;

    auto content = std::make_unique<SettingsWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"Settings", offsetX, offsetY, w, h);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) { return; }

    if (m_pMouse)
    {
        pContent->SetDependencies(*pWindow, m_themeManager, *m_pMouse);
    }

    SettingsInstance inst;
    inst.pWindow  = pWindow;
    inst.pContent = std::move(content);
    m_instances.push_back(std::move(inst));

    m_windowManager.SetFocusedWindow(pWindow);
}

void SettingsSystem::CloseSettings(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_instances.begin(), m_instances.end(),
        [windowId](const SettingsInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_instances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_instances.erase(it);
    }
}

void SettingsSystem::RemoveClosedWindows() noexcept
{
    for (auto it = m_instances.begin(); it != m_instances.end(); )
    {
        if (!it->pWindow || !it->pWindow->IsVisible())
        {
            m_windowManager.RemoveWindow(it->pWindow);
            it = m_instances.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace
