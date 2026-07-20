#pragma once

#include <Engine/System.hpp>
#include <Explorer/ExplorerWindow.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace DragonOS::Theme           { class ThemeManager; }
namespace DragonOS::Input           { class MouseManager; }
namespace DragonOS::Animation       { class AnimationManager; }
namespace DragonOS::WindowManager   { class WindowManager; class DragonWindow; }

namespace DragonOS::FileSystem { class FileSystemService; }

namespace DragonOS::Explorer {

class ExplorerSystem final : public Engine::System {
public:
    ExplorerSystem(
        WindowManager::WindowManager& wndMgr,
        Theme::ThemeManager&          themeMgr,
        Animation::AnimationManager&  animMgr,
        FileSystem::FileSystemService& fsService) noexcept;

    ~ExplorerSystem() noexcept { Shutdown(); }

    ExplorerSystem(const ExplorerSystem&)            = delete;
    ExplorerSystem& operator=(const ExplorerSystem&) = delete;
    ExplorerSystem(ExplorerSystem&&)                 = delete;
    ExplorerSystem& operator=(ExplorerSystem&&)      = delete;

    // ── Engine::System ───────────────────────────────────────────────────

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    // ── Explorer management ──────────────────────────────────────────────

    /// @brief  Open a new Explorer window.
    void OpenExplorer() noexcept;

    /// @brief  Close an Explorer window by DragonWindow ID.
    void CloseExplorer(uint64_t windowId) noexcept;

    /// @brief  Get the number of open Explorer windows.
    [[nodiscard]] size_t GetWindowCount() const noexcept { return m_instances.size(); }

    /// @brief  Set the MouseManager (called during engine wiring).
    void SetMouseManager(Input::MouseManager& mouseMgr) noexcept
    {
        m_pMouse = &mouseMgr;
    }

    [[nodiscard]] Input::MouseManager* GetMouseManager() const noexcept
    {
        return m_pMouse;
    }

private:
    struct ExplorerInstance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<Explorer::ExplorerWindow> pContent;
    };

    void RemoveClosedWindows() noexcept;

    std::vector<ExplorerInstance> m_instances;

    WindowManager::WindowManager&    m_windowManager;
    Theme::ThemeManager&             m_themeManager;
    Animation::AnimationManager&     m_animManager;
    FileSystem::FileSystemService&   m_fsService;
    Input::MouseManager*             m_pMouse{ nullptr };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
    bool  m_initialized{ false };
};

} // namespace DragonOS::Explorer
