#include <Explorer/ExplorerSystem.hpp>
#include <Explorer/ExplorerWindow.hpp>

#include <Animation/AnimationManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Explorer {

// ============================================================================
//  Construction
// ============================================================================

ExplorerSystem::ExplorerSystem(
    WindowManager::WindowManager& wndMgr,
    Theme::ThemeManager&          themeMgr,
    Animation::AnimationManager&  animMgr,
    FileSystem::FileSystemService& fsService) noexcept
    : m_windowManager{ wndMgr }
    , m_themeManager{ themeMgr }
    , m_animManager{ animMgr }
    , m_fsService{ fsService }
{
}

// ============================================================================
//  Engine::System
// ============================================================================

bool ExplorerSystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) { return true; }

    m_viewportWidth  = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_initialized = true;
    return true;
}

void ExplorerSystem::Shutdown() noexcept
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

void ExplorerSystem::Update(float /*deltaTime*/) noexcept
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

void ExplorerSystem::Render(Engine::EngineContext& ctx) noexcept
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

void ExplorerSystem::Resize(float width, float height) noexcept
{
    m_viewportWidth  = width;
    m_viewportHeight = height;
}

// ============================================================================
//  Explorer management
// ============================================================================

void ExplorerSystem::OpenExplorer() noexcept
{
    if (!m_initialized) { return; }

    // Cascade window positions
    const float offsetX = 40.0f + (m_instances.size() % 5) * 30.0f;
    const float offsetY = 40.0f + (m_instances.size() % 5) * 30.0f;
    const float w = 900.0f;
    const float h = 600.0f;

    auto content = std::make_unique<Explorer::ExplorerWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"Explorer", offsetX, offsetY, w, h);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) { return; }

    // Initialize content
    if (m_pMouse)
    {
        pContent->SetDependencies(*pWindow, m_fsService, m_themeManager, *m_pMouse);
    }

    ExplorerInstance inst;
    inst.pWindow  = pWindow;
    inst.pContent = std::move(content);
    m_instances.push_back(std::move(inst));

    // Focus the new window
    m_windowManager.SetFocusedWindow(pWindow);
}

void ExplorerSystem::CloseExplorer(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_instances.begin(), m_instances.end(),
        [windowId](const ExplorerInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_instances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_instances.erase(it);
    }
}

void ExplorerSystem::RemoveClosedWindows() noexcept
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

} // namespace DragonOS::Explorer
