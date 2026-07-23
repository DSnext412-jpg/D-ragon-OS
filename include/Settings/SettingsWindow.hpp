#pragma once
#include <Settings/SettingsTypes.hpp>
#include <UI/UI.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class MouseManager; }
namespace DragonOS::WindowManager { class DragonWindow; }

namespace DragonOS::Settings {

class SettingsWindow final {
public:
    SettingsWindow() noexcept;
    ~SettingsWindow() noexcept = default;

    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;
    SettingsWindow(SettingsWindow&&) = delete;
    SettingsWindow& operator=(SettingsWindow&&) = delete;

    void SetDependencies(
        WindowManager::DragonWindow& window,
        Theme::ThemeManager& themeManager,
        Input::MouseManager& mouseManager) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept { m_pWindow = &window; }

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;
    void ProcessInput() noexcept;

    [[nodiscard]] uint64_t GetWindowId() const noexcept;
    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"Settings";
        return name;
    }

private:
    void BuildUI() noexcept;
    void OnCategoryChanged(int index) noexcept;
    UI::UIRenderer MakeUIRenderer(Graphics::Renderer& renderer) const noexcept;

    std::unique_ptr<UI::TabControl> m_tabControl;
    std::vector<std::unique_ptr<UI::StackPanel>> m_categoryPages;
    std::unique_ptr<UI::StatusBar> m_statusBar;

    WindowManager::DragonWindow* m_pWindow{nullptr};
    Theme::ThemeManager*         m_pTheme{nullptr};
    Input::MouseManager*         m_pMouse{nullptr};

    int     m_selectedCategory{0};
    float   m_viewportWidth{0}, m_viewportHeight{0};
    bool    m_initialized{false};
};

} // namespace
