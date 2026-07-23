#pragma once

#include <Diagnostics/DiagnosticsTypes.hpp>

#include <Graphics/Color.hpp>
#include <Input/HitTest.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; class MouseManager; }
namespace DragonOS::WindowManager { class DragonWindow; }

namespace DragonOS::Diagnostics {

class DebugConsoleWindow final {
public:
    DebugConsoleWindow() noexcept;
    ~DebugConsoleWindow() noexcept;

    DebugConsoleWindow(const DebugConsoleWindow&) = delete;
    DebugConsoleWindow& operator=(const DebugConsoleWindow&) = delete;
    DebugConsoleWindow(DebugConsoleWindow&&) = delete;
    DebugConsoleWindow& operator=(DebugConsoleWindow&&) = delete;

    void SetDependencies(
        WindowManager::DragonWindow& window,
        Theme::ThemeManager& themeManager,
        Input::InputManager& inputManager,
        Input::MouseManager& mouseManager) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept
    {
        m_pWindow = &window;
    }

    void AppendLog(const LogEntry& entry) noexcept;
    void Clear() noexcept;

    void SetLogLevelFilter(LogLevel minLevel) noexcept { m_minLevel = minLevel; }
    [[nodiscard]] LogLevel GetLogLevelFilter() const noexcept { return m_minLevel; }

    void SetSearchFilter(std::wstring_view text) noexcept { m_searchFilter = text; }
    [[nodiscard]] const std::wstring& GetSearchFilter() const noexcept { return m_searchFilter; }

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;
    void ProcessKeyboardInput(Input::InputManager& inputManager) noexcept;

    [[nodiscard]] uint64_t GetWindowId() const noexcept;

    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"Debug Console";
        return name;
    }

private:
    void RecalculateLayout() noexcept;

    void RenderToolbar(Graphics::Renderer& renderer) noexcept;
    void RenderLogEntries(Graphics::Renderer& renderer) noexcept;
    void RenderInputLine(Graphics::Renderer& renderer) noexcept;
    void RenderStatusBar(Graphics::Renderer& renderer) noexcept;

    void ExecuteCommand() noexcept;

    [[nodiscard]] Input::Bounds GetLogAreaBounds() const noexcept;
    [[nodiscard]] Input::Bounds GetInputBounds() const noexcept;

    [[nodiscard]] Graphics::Color GetLevelColor(LogLevel level) const noexcept;

    struct DisplayEntry {
        const LogEntry* pEntry{ nullptr };
        bool filtered{ false };
    };

    std::vector<DisplayEntry> m_displayEntries;
    size_t m_maxDisplayEntries{ 1000 };
    LogLevel m_minLevel{ LogLevel::Trace };
    std::wstring m_searchFilter;
    std::wstring m_inputLine;
    size_t m_cursorPos{ 0 };
    bool m_autoScroll{ true };
    float m_scrollOffset{ 0.0f };
    float m_scrollTarget{ 0.0f };

    float m_viewportWidth{ 0.0f };
    float m_viewportHeight{ 0.0f };

    struct Layout {
        Input::Bounds toolbarArea{};
        Input::Bounds logArea{};
        Input::Bounds inputArea{};
        Input::Bounds statusBarArea{};
        float padding{ 4.0f };
        float toolbarHeight{ 28.0f };
        float inputLineHeight{ 24.0f };
        float statusBarHeight{ 20.0f };
    };
    Layout m_layout{};

    WindowManager::DragonWindow* m_pWindow{ nullptr };
    Theme::ThemeManager* m_pTheme{ nullptr };
    Input::InputManager* m_pInput{ nullptr };
    Input::MouseManager* m_pMouse{ nullptr };

    bool m_initialized{ false };
};

} // namespace DragonOS::Diagnostics
