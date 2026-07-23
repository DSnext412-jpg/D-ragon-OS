#pragma once

#include <Command/CommandRegistry.hpp>
#include <Command/CommandResult.hpp>
#include <History/History.hpp>
#include <Terminal/TerminalBuffer.hpp>
#include <Terminal/TerminalTheme.hpp>

#include <Input/HitTest.hpp>

#include <d2d1.h>
#include <dwrite.h>

#include <UI/UI.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace DragonOS::Graphics { class Renderer; }
namespace DragonOS::Theme { class ThemeManager; }
namespace DragonOS::Input { class InputManager; class MouseManager; }
namespace DragonOS::WindowManager { class DragonWindow; }
namespace DragonOS::FileSystem { class FileSystemService; }

namespace DragonOS::Terminal {

class TerminalWindow final {
public:
    TerminalWindow() noexcept;
    ~TerminalWindow() noexcept;

    TerminalWindow(const TerminalWindow&) = delete;
    TerminalWindow& operator=(const TerminalWindow&) = delete;
    TerminalWindow(TerminalWindow&&) = delete;
    TerminalWindow& operator=(TerminalWindow&&) = delete;

    void SetDependencies(
        WindowManager::DragonWindow&  window,
        Theme::ThemeManager&          themeManager,
        Input::InputManager&          inputManager,
        Input::MouseManager&          mouseManager,
        FileSystem::FileSystemService& fsService) noexcept;

    void SetWindow(WindowManager::DragonWindow& window) noexcept { m_pWindow = &window; }

    void Update() noexcept;
    void Render(Graphics::Renderer& renderer) noexcept;

    void ProcessKeyboardInput(
        Input::InputManager& inputManager) noexcept;

    [[nodiscard]] uint64_t GetWindowId() const noexcept;

    [[nodiscard]] static const std::wstring& GetAppName() noexcept
    {
        static const std::wstring name = L"Terminal";
        return name;
    }

    void WriteOutput(const std::wstring& text) noexcept;
    void WriteLine(const std::wstring& text) noexcept;

private:
    void InitializeDirectWrite() noexcept;
    void RecalculateLayout() noexcept;

    void RenderOutput(Graphics::Renderer& renderer) noexcept;
    void RenderInputLine(Graphics::Renderer& renderer) noexcept;
    void RenderStatusLine(Graphics::Renderer& renderer) noexcept;
    void RenderScrollbar(Graphics::Renderer& renderer) noexcept;

    UI::UIRenderer MakeUIRenderer(Graphics::Renderer& renderer) const noexcept;

    void ExecuteCommand() noexcept;
    void HandleInput() noexcept;

    void InsertChar(wchar_t ch) noexcept;
    void DeleteChar() noexcept;
    void MoveCursorLeft() noexcept;
    void MoveCursorRight() noexcept;
    void MoveCursorHome() noexcept;
    void MoveCursorEnd() noexcept;

    void ScrollUp() noexcept;
    void ScrollDown() noexcept;
    void ScrollToBottom() noexcept;

    [[nodiscard]] Input::Bounds GetOutputBounds() const noexcept;
    [[nodiscard]] Input::Bounds GetInputBounds() const noexcept;
    [[nodiscard]] Input::Bounds GetStatusBounds() const noexcept;

    void PrintPrompt() noexcept;

    TerminalBuffer               m_buffer;
    Command::CommandRegistry     m_commandRegistry;
    History::History             m_history;

    std::wstring                 m_inputLine;
    size_t                       m_cursorPos{ 0 };

    std::wstring                 m_currentDirectory;
    std::wstring                 m_promptString;

    TerminalColors               m_colors;

    IDWriteFactory*              m_pDWriteFactory{ nullptr };
    IDWriteTextFormat*           m_pTextFormat{ nullptr };

    float                        m_charWidth{ 0.0f };
    float                        m_charHeight{ 0.0f };
    float                        m_viewportWidth{ 0.0f };
    float                        m_viewportHeight{ 0.0f };

    struct Layout {
        Input::Bounds outputArea{};
        Input::Bounds inputArea{};
        Input::Bounds statusArea{};
        float          scrollbarWidth{ 8.0f };
        float          statusLineHeight{ 20.0f };
        float          padding{ 4.0f };
    };
    Layout m_layout{};

    size_t                       m_visibleLines{ 0 };
    size_t                       m_visibleCols{ 0 };

    bool                          m_scrollAtBottom{ true };
    float                         m_scrollbarHoverAnim{ 0.0f };
    bool                          m_scrollbarHovered{ false };

    WindowManager::DragonWindow*  m_pWindow{ nullptr };
    Theme::ThemeManager*           m_pTheme{ nullptr };
    Input::InputManager*           m_pInput{ nullptr };
    Input::MouseManager*           m_pMouse{ nullptr };
    FileSystem::FileSystemService* m_pFS{ nullptr };

    bool                          m_initialized{ false };

    // UI Framework integration
    std::unique_ptr<UI::StatusBar> m_uiStatusBar;
};

} // namespace DragonOS::Terminal
