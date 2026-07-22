#pragma once

#include <Engine/System.hpp>
#include <Terminal/TerminalWindow.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; class MouseManager; }
namespace DragonOS::Animation { class AnimationManager; }
namespace DragonOS::WindowManager { class WindowManager; class DragonWindow; }
namespace DragonOS::FileSystem { class FileSystemService; }

namespace DragonOS::Terminal {

class TerminalSystem final : public Engine::System {
public:
    TerminalSystem(
        WindowManager::WindowManager& wndMgr,
        Theme::ThemeManager&          themeMgr,
        Animation::AnimationManager&  animMgr,
        FileSystem::FileSystemService& fsService,
        Input::InputManager&          inputMgr) noexcept;

    ~TerminalSystem() noexcept { Shutdown(); }

    TerminalSystem(const TerminalSystem&) = delete;
    TerminalSystem& operator=(const TerminalSystem&) = delete;
    TerminalSystem(TerminalSystem&&) = delete;
    TerminalSystem& operator=(TerminalSystem&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void OpenTerminal() noexcept;
    void CloseTerminal(uint64_t windowId) noexcept;
    size_t GetWindowCount() const noexcept { return m_instances.size(); }

    void SetMouseManager(Input::MouseManager& mouseMgr) noexcept
    {
        m_pMouse = &mouseMgr;
    }

    [[nodiscard]] Input::MouseManager* GetMouseManager() const noexcept
    {
        return m_pMouse;
    }

private:
    struct TerminalInstance final {
        WindowManager::DragonWindow* pWindow{ nullptr };
        std::unique_ptr<TerminalWindow> pContent;
    };

    void RemoveClosedWindows() noexcept;

    std::vector<TerminalInstance> m_instances;

    WindowManager::WindowManager&    m_windowManager;
    Theme::ThemeManager&             m_themeManager;
    Animation::AnimationManager&     m_animManager;
    FileSystem::FileSystemService&   m_fsService;
    Input::InputManager&             m_inputManager;
    Input::MouseManager*             m_pMouse{ nullptr };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };
    bool  m_initialized{ false };
};

} // namespace DragonOS::Terminal
