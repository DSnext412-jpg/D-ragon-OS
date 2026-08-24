#include <Explorer2/ExplorerSystem.hpp>

#include <Animation/AnimationManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/InputManager.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <WindowManager/DragonWindow.hpp>
#include <WindowManager/WindowManager.hpp>

#include <algorithm>

namespace DragonOS::Explorer2 {

using WindowManager::DragonWindow;

// ============================================================================
//  Plugin host bridge
// ============================================================================

void ExplorerSystem::PluginHost::AddContextMenuExtension(
    IExplorerPlugin* /*owner*/, IExplorerContextMenuExtension* extension)
{
    m_owner.m_contextExtensions.push_back(extension);
    for (Instance& inst : m_owner.m_instances)
    {
        if (inst.pContent) { inst.pContent->AddContextMenuExtension(extension); }
    }
}

void ExplorerSystem::PluginHost::AddPreviewProvider(
    IExplorerPlugin* /*owner*/, IPreviewProvider* provider)
{
    m_owner.m_previewProviders.push_back(provider);
    for (Instance& inst : m_owner.m_instances)
    {
        if (inst.pContent) { inst.pContent->AddPreviewProvider(provider); }
    }
}

void ExplorerSystem::PluginHost::AddOpenWithHandler(
    IExplorerPlugin* /*owner*/, const std::wstring& dotExtension,
    const std::wstring& label,
    std::function<void(const FileSystem::FileEntry&)> action)
{
    for (Instance& inst : m_owner.m_instances)
    {
        if (inst.pContent)
        {
            inst.pContent->AddOpenWithHandler(dotExtension, label, action);
        }
    }
}

void ExplorerSystem::PluginHost::Announce(const std::wstring& text)
{
    for (Instance& inst : m_owner.m_instances)
    {
        if (inst.pContent) { inst.pContent->Announce(text); }
    }
}

// ============================================================================
//  Construction
// ============================================================================

ExplorerSystem::ExplorerSystem(
    WindowManager::WindowManager& windowManager,
    Theme::ThemeManager& themeManager,
    Animation::AnimationManager& animationManager,
    FileSystem::FileSystemService& fileSystemService) noexcept
    : m_windowManager{ windowManager }
    , m_themeManager{ themeManager }
    , m_animManager{ animationManager }
    , m_fsService{ fileSystemService }
    , m_pluginHost{ *this }
{
}

ExplorerSystem::~ExplorerSystem() noexcept
{
    Shutdown();
}

// ============================================================================
//  Engine::System
// ============================================================================

bool ExplorerSystem::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) { return true; }

    m_viewportWidth = ctx.GetViewportWidth();
    m_viewportHeight = ctx.GetViewportHeight();

    m_initialized = true;
    return true;
}

void ExplorerSystem::Shutdown() noexcept
{
    if (!m_initialized) { return; }

    for (auto& plugin : m_plugins)
    {
        plugin->OnUnregistered();
    }

    for (auto& inst : m_instances)
    {
        if (inst.pWindow)
        {
            m_windowManager.RemoveWindow(inst.pWindow);
        }
    }
    m_instances.clear();

    m_contextExtensions.clear();
    m_previewProviders.clear();
    m_plugins.clear();

    m_cache.Clear();
    m_initialized = false;
}

void ExplorerSystem::Update(float deltaTime) noexcept
{
    if (!m_initialized) { return; }

    RemoveClosedWindows();
    DispatchInputEvents();

    for (auto& inst : m_instances)
    {
        if (!inst.pWindow || !inst.pContent || !inst.pWindow->IsVisible())
        {
            continue;
        }

        // Client rect mirrors the OS window chrome contract.
        const float border = Theme::ThemeMetrics::WindowBorderThickness;
        const float titleH = Theme::ThemeMetrics::TitleBarHeight;
        const float clientX = inst.pWindow->GetX() + border;
        const float clientY = inst.pWindow->GetY() + titleH;
        const float clientW = inst.pWindow->GetWidth() - border * 2.0f;
        const float clientH = inst.pWindow->GetHeight() - titleH - border;

        inst.pContent->Layout(clientX, clientY, clientW, clientH,
                              m_viewportWidth, m_viewportHeight);
        inst.pContent->Update(deltaTime);
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
    m_viewportWidth = width;
    m_viewportHeight = height;
}

// ============================================================================
//  Input dispatch
// ============================================================================

namespace {

[[nodiscard]] bool PointInWindow(const DragonWindow& window, float x, float y) noexcept
{
    return x >= window.GetX() && x < window.GetX() + window.GetWidth()
        && y >= window.GetY() && y < window.GetY() + window.GetHeight();
}

} // namespace

void ExplorerSystem::DispatchInputEvents() noexcept
{
    if (!m_input || m_instances.empty()) { return; }

    DragonWindow* focused = m_windowManager.GetFocusedWindow();
    Instance* focusedInstance = nullptr;
    for (auto& inst : m_instances)
    {
        if (inst.pWindow == focused)
        {
            focusedInstance = &inst;
            break;
        }
    }

    for (const Input::InputEvent& event : m_input->GetEvents())
    {
        switch (event.type)
        {
        case Input::EventType::MouseMove:
        case Input::EventType::MouseDown:
        case Input::EventType::MouseUp:
        {
            // Route mouse to the focused window when it contains the point,
            // otherwise to the top-most window under the cursor.
            Instance* target = focusedInstance;
            const float x = event.data.mouseButton.x;
            const float y = event.data.mouseButton.y;
            if (!(target && target->pWindow
                  && PointInWindow(*target->pWindow, x, y)))
            {
                target = nullptr;
                for (auto it = m_instances.rbegin(); it != m_instances.rend(); ++it)
                {
                    if (it->pWindow && PointInWindow(*it->pWindow, x, y))
                    {
                        target = &*it;
                        break;
                    }
                }
            }
            if (!target || !target->pContent) { break; }

            switch (event.type)
            {
            case Input::EventType::MouseMove:
                target->pContent->HandleMouseMove(x, y);
                break;
            case Input::EventType::MouseDown:
                target->pContent->HandleMouseDown(
                    x, y, event.data.mouseButton.button);
                break;
            default:
                target->pContent->HandleMouseUp(
                    x, y, event.data.mouseButton.button);
                break;
            }
            break;
        }

        case Input::EventType::MouseWheel:
        {
            Instance* target = nullptr;
            for (auto it = m_instances.rbegin(); it != m_instances.rend(); ++it)
            {
                if (it->pWindow
                    && PointInWindow(*it->pWindow,
                                     event.data.mouseWheel.x, event.data.mouseWheel.y))
                {
                    target = &*it;
                    break;
                }
            }
            if (target && target->pContent)
            {
                target->pContent->HandleMouseWheel(
                    event.data.mouseWheel.delta,
                    event.data.mouseWheel.x, event.data.mouseWheel.y);
            }
            break;
        }

        case Input::EventType::KeyDown:
        case Input::EventType::KeyUp:
            if (focusedInstance && focusedInstance->pContent)
            {
                using IK = Input::KeyCode;
                const bool ctrl =
                    m_input->IsKeyHeld(IK::LControl) || m_input->IsKeyHeld(IK::RControl);
                const bool shift =
                    m_input->IsKeyHeld(IK::LShift) || m_input->IsKeyHeld(IK::RShift);
                const bool alt =
                    m_input->IsKeyHeld(IK::LAlt) || m_input->IsKeyHeld(IK::RAlt);
                if (event.type == Input::EventType::KeyDown)
                {
                    focusedInstance->pContent->HandleKey(
                        event.data.key.key, ctrl, shift, alt, event.data.key.isRepeat);
                }
            }
            break;

        case Input::EventType::CharacterInput:
            if (focusedInstance && focusedInstance->pContent)
            {
                focusedInstance->pContent->HandleCharacter(event.data.character.character);
            }
            break;

        default:
            break;
        }
    }
}

// ============================================================================
//  Window management
// ============================================================================

void ExplorerSystem::OpenExplorer() noexcept
{
    if (!m_initialized) { return; }

    // Cascade positions like the legacy system.
    const float offset = static_cast<float>(m_instances.size() % 5) * 30.0f;
    const float x = 40.0f + offset;
    const float y = 40.0f + offset;
    constexpr float kDefaultWidth = 980.0f;
    constexpr float kDefaultHeight = 640.0f;

    auto content = std::make_unique<ExplorerWindow>();
    ExplorerWindow* pContent = content.get();

    auto window = std::make_unique<DragonWindow>(
        L"Explorer", x, y, kDefaultWidth, kDefaultHeight);
    window->Open();
    DragonWindow* pWindow = m_windowManager.AddWindow(std::move(window));
    if (!pWindow || !pContent) { return; }

    pContent->SetWindow(*pWindow);

    ExplorerWindow::Services services;
    services.fs = &m_fsService;
    services.theme = &m_themeManager;
    services.mouse = m_mouse;
    services.cache = &m_cache;
    pContent->SetServices(services);

    // Replay existing plugin contributions onto the new window.
    for (IExplorerContextMenuExtension* extension : m_contextExtensions)
    {
        pContent->AddContextMenuExtension(extension);
    }
    for (IPreviewProvider* provider : m_previewProviders)
    {
        pContent->AddPreviewProvider(provider);
    }

    m_instances.push_back({ pWindow, std::move(content) });
    m_windowManager.SetFocusedWindow(pWindow);
}

void ExplorerSystem::CloseExplorer(uint64_t windowId) noexcept
{
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
    {
        if (it->pWindow && it->pWindow->GetId() == windowId)
        {
            m_windowManager.RemoveWindow(it->pWindow);
            m_instances.erase(it);
            return;
        }
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

// ============================================================================
//  Plugins
// ============================================================================

void ExplorerSystem::RegisterPlugin(std::unique_ptr<IExplorerPlugin> plugin) noexcept
{
    if (!plugin) { return; }
    plugin->OnRegistered(m_pluginHost);
    m_plugins.push_back(std::move(plugin));
}

} // namespace DragonOS::Explorer2
