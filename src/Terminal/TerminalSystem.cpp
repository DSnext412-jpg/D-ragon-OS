#include <Terminal/TerminalSystem.hpp>
#include <Terminal/TerminalWindow.hpp>

#include <Animation/AnimationManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/InputManager.hpp>
#include <Input/Keyboard.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <WindowManager/WindowManager.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::Terminal {

// ============================================================================
//  Construction
// ============================================================================

TerminalSystem::TerminalSystem(
    WindowManager::WindowManager& wndMgr,
    Theme::ThemeManager&          themeMgr,
    Animation::AnimationManager&  animMgr,
    FileSystem::FileSystemService& fsService,
    Input::InputManager&          inputMgr) noexcept
    : m_windowManager{ wndMgr }
    , m_themeManager{ themeMgr }
    , m_animManager{ animMgr }
    , m_fsService{ fsService }
    , m_inputManager{ inputMgr }
{
}

// ============================================================================
//  Engine::System
// ============================================================================

bool TerminalSystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) { return true; }

    m_viewportWidth  = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_initialized = true;
    return true;
}

void TerminalSystem::Shutdown() noexcept
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

void TerminalSystem::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    RemoveClosedWindows();

    for (auto& inst : m_instances)
    {
        if (inst.pContent && inst.pWindow && inst.pWindow->IsVisible())
        {
            inst.pContent->Update();

            if (inst.pWindow->IsFocused())
            {
                inst.pContent->ProcessKeyboardInput(m_inputManager);
            }
        }
    }

    (void)deltaTime;
}

void TerminalSystem::Render(Engine::EngineContext& ctx) noexcept
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

void TerminalSystem::Resize(float width, float height) noexcept
{
    m_viewportWidth  = width;
    m_viewportHeight = height;
}

// ============================================================================
//  Terminal management
// ============================================================================

void TerminalSystem::OpenTerminal() noexcept
{
    if (!m_initialized) { return; }

    const float offsetX = 60.0f + (m_instances.size() % 5) * 30.0f;
    const float offsetY = 60.0f + (m_instances.size() % 5) * 30.0f;
    const float w = 800.0f;
    const float h = 500.0f;

    auto content = std::make_unique<TerminalWindow>();
    auto* pContent = content.get();

    auto window = std::make_unique<WindowManager::DragonWindow>(
        L"Terminal", offsetX, offsetY, w, h);
    window->Open();
    auto* pWindow = m_windowManager.AddWindow(std::move(window));

    if (!pWindow || !pContent) { return; }

    if (m_pMouse)
    {
        pContent->SetDependencies(*pWindow, m_themeManager, m_inputManager, *m_pMouse, m_fsService);
    }

    TerminalInstance inst;
    inst.pWindow  = pWindow;
    inst.pContent = std::move(content);
    m_instances.push_back(std::move(inst));

    m_windowManager.SetFocusedWindow(pWindow);
}

void TerminalSystem::CloseTerminal(uint64_t windowId) noexcept
{
    auto it = std::find_if(m_instances.begin(), m_instances.end(),
        [windowId](const TerminalInstance& inst) {
            return inst.pWindow && inst.pWindow->GetId() == windowId;
        });

    if (it != m_instances.end())
    {
        m_windowManager.RemoveWindow(it->pWindow);
        m_instances.erase(it);
    }
}

void TerminalSystem::RemoveClosedWindows() noexcept
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

} // namespace DragonOS::Terminal
