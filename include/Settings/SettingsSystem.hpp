#pragma once
#include <Engine/System.hpp>
#include <Settings/SettingsWindow.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace DragonOS::Theme         { class ThemeManager; }
namespace DragonOS::Input         { class MouseManager; }
namespace DragonOS::Animation     { class AnimationManager; }
namespace DragonOS::WindowManager { class WindowManager; class DragonWindow; }

namespace DragonOS::Settings {

class SettingsSystem final : public Engine::System {
public:
    SettingsSystem(
        WindowManager::WindowManager& wndMgr,
        Theme::ThemeManager& themeMgr) noexcept;

    ~SettingsSystem() noexcept { Shutdown(); }

    SettingsSystem(const SettingsSystem&) = delete;
    SettingsSystem& operator=(const SettingsSystem&) = delete;
    SettingsSystem(SettingsSystem&&) = delete;
    SettingsSystem& operator=(SettingsSystem&&) = delete;

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    void OpenSettings() noexcept;
    void CloseSettings(uint64_t windowId) noexcept;
    [[nodiscard]] size_t GetWindowCount() const noexcept { return m_instances.size(); }
    void SetMouseManager(Input::MouseManager& mouseMgr) noexcept { m_pMouse = &mouseMgr; }

private:
    struct SettingsInstance {
        WindowManager::DragonWindow* pWindow{nullptr};
        std::unique_ptr<SettingsWindow> pContent;
    };

    void RemoveClosedWindows() noexcept;

    std::vector<SettingsInstance> m_instances;
    WindowManager::WindowManager& m_windowManager;
    Theme::ThemeManager&          m_themeManager;
    Input::MouseManager*          m_pMouse{nullptr};
    float m_viewportWidth{0}, m_viewportHeight{0};
    bool  m_initialized{false};
};

} // namespace
