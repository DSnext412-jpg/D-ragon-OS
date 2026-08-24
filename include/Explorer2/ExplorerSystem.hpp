// ============================================================================
//  ExplorerSystem.hpp — Engine system hosting any number of Explorer windows.
//
//  Replaces the legacy Explorer::ExplorerSystem with the identical hosting
//  contract plus:
//    - InputManager routing (keyboard → focused window, mouse → hit test),
//    - a shared DirectoryCache across every window,
//    - a plugin registry (IExplorerPlugin) feeding all current/future windows.
// ============================================================================

#pragma once

#include <Engine/System.hpp>
#include <Explorer2/DirectoryCache.hpp>
#include <Explorer2/ExplorerPluginAPI.hpp>
#include <Explorer2/ExplorerWindow.hpp>
#include <FileSystem/FileSystemService.hpp>

#include <memory>
#include <vector>

namespace DragonOS {

namespace WindowManager { class WindowManager; class DragonWindow; }
namespace Theme { class ThemeManager; }
namespace Animation { class AnimationManager; }
namespace Input { class MouseManager; class InputManager; }

namespace Explorer2 {

class ExplorerSystem final : public Engine::System {
public:
    ExplorerSystem(
        WindowManager::WindowManager& windowManager,
        Theme::ThemeManager& themeManager,
        Animation::AnimationManager& animationManager,
        FileSystem::FileSystemService& fileSystemService) noexcept;

    ~ExplorerSystem() noexcept override;

    // ── Engine::System ──────────────────────────────────────────────────

    [[nodiscard]] bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    // ── Wiring (called by the engine right after registration) ──────────

    void SetMouseManager(Input::MouseManager* mouse) noexcept { m_mouse = mouse; }
    void SetInputManager(Input::InputManager* input) noexcept { m_input = input; }

    // ── Window management ───────────────────────────────────────────────

    void OpenExplorer() noexcept;
    void CloseExplorer(uint64_t windowId) noexcept;

    // ── Plugins ─────────────────────────────────────────────────────────

    /// Takes ownership; OnRegistered fires immediately for existing windows
    /// and again for each window opened afterwards.
    void RegisterPlugin(std::unique_ptr<IExplorerPlugin> plugin) noexcept;

    [[nodiscard]] size_t GetWindowCount() const noexcept { return m_instances.size(); }

private:
    struct Instance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<ExplorerWindow> pContent;
    };

    /// Bridges IExplorerPluginHost to this system's registry + windows.
    class PluginHost final : public IExplorerPluginHost {
    public:
        explicit PluginHost(ExplorerSystem& owner) noexcept : m_owner(owner) {}

        void AddContextMenuExtension(IExplorerPlugin* owner,
                                     IExplorerContextMenuExtension* extension) override;
        void AddPreviewProvider(IExplorerPlugin* owner, IPreviewProvider* provider) override;
        void AddOpenWithHandler(IExplorerPlugin* owner, const std::wstring& dotExtension,
                                const std::wstring& label,
                                std::function<void(const FileSystem::FileEntry&)> action) override;
        void Announce(const std::wstring& text) override;

    private:
        ExplorerSystem& m_owner;
    };

    void RemoveClosedWindows() noexcept;
    void DispatchInputEvents() noexcept;
    [[nodiscard]] Instance* FindInstanceForWindow(uint64_t windowId) noexcept;

    WindowManager::WindowManager& m_windowManager;
    Theme::ThemeManager& m_themeManager;
    Animation::AnimationManager& m_animManager;
    FileSystem::FileSystemService& m_fsService;

    DirectoryCache m_cache;
    std::vector<Instance> m_instances;

    PluginHost m_pluginHost;
    std::vector<std::unique_ptr<IExplorerPlugin>> m_plugins;
    std::vector<IExplorerContextMenuExtension*> m_contextExtensions;
    std::vector<IPreviewProvider*> m_previewProviders;

    Input::MouseManager* m_mouse{ nullptr };
    Input::InputManager* m_input{ nullptr };

    float m_viewportWidth{ 1280.0f };
    float m_viewportHeight{ 720.0f };
    bool m_initialized{ false };
};

} // namespace Explorer2
} // namespace DragonOS
