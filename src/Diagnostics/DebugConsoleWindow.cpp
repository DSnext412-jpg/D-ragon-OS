#include <Diagnostics/DebugConsoleWindow.hpp>

#include <Graphics/Renderer.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemePalette.hpp>
#include <Input/InputManager.hpp>
#include <Input/MouseManager.hpp>
#include <WindowManager/DragonWindow.hpp>

namespace DragonOS::Diagnostics {

DebugConsoleWindow::DebugConsoleWindow() noexcept
{
    m_displayEntries.reserve(m_maxDisplayEntries);
}

DebugConsoleWindow::~DebugConsoleWindow() noexcept
{
}

void DebugConsoleWindow::SetDependencies(
    WindowManager::DragonWindow& window,
    Theme::ThemeManager& themeManager,
    Input::InputManager& inputManager,
    Input::MouseManager& mouseManager) noexcept
{
    m_pWindow = &window;
    m_pTheme = &themeManager;
    m_pInput = &inputManager;
    m_pMouse = &mouseManager;
}

void DebugConsoleWindow::AppendLog(const LogEntry& entry) noexcept
{
    if (m_displayEntries.size() >= m_maxDisplayEntries)
    {
        m_displayEntries.erase(m_displayEntries.begin());
    }

    DisplayEntry dispEntry;
    dispEntry.pEntry = &entry;
    dispEntry.filtered = false;

    if (entry.level < m_minLevel)
    {
        dispEntry.filtered = true;
    }
    else if (!m_searchFilter.empty())
    {
        bool found = entry.message.find(m_searchFilter) != std::wstring::npos ||
                     entry.source.find(m_searchFilter) != std::wstring::npos;
        if (!found)
        {
            dispEntry.filtered = true;
        }
    }

    m_displayEntries.push_back(dispEntry);

    if (m_autoScroll)
    {
        m_scrollOffset = 0.0f;
        m_scrollTarget = 0.0f;
    }
}

void DebugConsoleWindow::Clear() noexcept
{
    m_displayEntries.clear();
}

void DebugConsoleWindow::Update() noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    auto bounds = m_pWindow->GetBounds();
    m_viewportWidth = bounds.width;
    m_viewportHeight = bounds.height;

    RecalculateLayout();
}

void DebugConsoleWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_pWindow || !m_pWindow->IsVisible()) return;

    RenderToolbar(renderer);
    RenderLogEntries(renderer);
    RenderInputLine(renderer);
    RenderStatusBar(renderer);
}

void DebugConsoleWindow::ProcessKeyboardInput(Input::InputManager& inputManager) noexcept
{
    if (!m_initialized || !m_pWindow) return;

    auto& keyboard = inputManager.GetKeyboard();

    for (wchar_t ch : inputManager.GetCharBuffer())
    {
        if (ch == L'\r')
        {
            ExecuteCommand();
        }
        else if (ch == L'\b')
        {
            if (m_cursorPos > 0)
            {
                m_inputLine.erase(m_cursorPos - 1, 1);
                m_cursorPos--;
            }
        }
        else if (ch >= 32)
        {
            m_inputLine.insert(m_cursorPos, 1, ch);
            m_cursorPos++;
        }
    }

    using Key = Input::KeyCode;

    if (keyboard.IsKeyReleased(Key::Left))
    {
        if (m_cursorPos > 0) m_cursorPos--;
    }
    else if (keyboard.IsKeyReleased(Key::Right))
    {
        if (m_cursorPos < m_inputLine.size()) m_cursorPos++;
    }
    else if (keyboard.IsKeyReleased(Key::Home))
    {
        m_cursorPos = 0;
    }
    else if (keyboard.IsKeyReleased(Key::End))
    {
        m_cursorPos = m_inputLine.size();
    }
    else if (keyboard.IsKeyReleased(Key::Delete))
    {
        if (m_cursorPos < m_inputLine.size())
        {
            m_inputLine.erase(m_cursorPos, 1);
        }
    }
}

uint64_t DebugConsoleWindow::GetWindowId() const noexcept
{
    return m_pWindow ? m_pWindow->GetId() : 0;
}

void DebugConsoleWindow::RecalculateLayout() noexcept
{
    float pad = m_layout.padding;
    float totalW = m_viewportWidth;
    float totalH = m_viewportHeight;

    m_layout.toolbarArea = { pad, pad, totalW - pad * 2.0f, m_layout.toolbarHeight };

    float inputY = totalH - pad - m_layout.inputLineHeight;
    m_layout.inputArea = { pad, inputY, totalW - pad * 2.0f, m_layout.inputLineHeight };

    float statusY = inputY - m_layout.statusBarHeight;
    m_layout.statusBarArea = { pad, statusY, totalW - pad * 2.0f, m_layout.statusBarHeight };

    float logY = m_layout.toolbarArea.y + m_layout.toolbarArea.height + pad;
    float logH = statusY - logY - pad;
    m_layout.logArea = { pad, logY, totalW - pad * 2.0f, logH };
}

void DebugConsoleWindow::RenderToolbar(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.toolbarArea.x, m_layout.toolbarArea.y,
                          m_layout.toolbarArea.x + m_layout.toolbarArea.width,
                          m_layout.toolbarArea.y + m_layout.toolbarArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.15f);

    D2D1_RECT_F titleBounds = { area.left + 4.0f, area.top + 2.0f, area.right - 4.0f, area.bottom - 2.0f };
    renderer.DrawText(L"Debug Console", titleBounds, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void DebugConsoleWindow::RenderLogEntries(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);

    D2D1_RECT_F area = { m_layout.logArea.x, m_layout.logArea.y,
                          m_layout.logArea.x + m_layout.logArea.width,
                          m_layout.logArea.y + m_layout.logArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.05f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    float lineH = 14.0f;
    float y = area.bottom - lineH + m_scrollOffset;

    int visibleCount = static_cast<int>((area.bottom - area.top) / lineH) + 1;
    int startIdx = static_cast<int>(m_displayEntries.size()) - visibleCount;
    if (startIdx < 0) startIdx = 0;

    for (int i = startIdx; i < static_cast<int>(m_displayEntries.size()); i++)
    {
        const auto& dispEntry = m_displayEntries[i];
        if (dispEntry.filtered || !dispEntry.pEntry) continue;

        D2D1_RECT_F lineBounds = { area.left + 2.0f, y, area.right - 2.0f, y + lineH };
        if (y + lineH >= area.top && y <= area.bottom)
        {
            auto color = GetLevelColor(dispEntry.pEntry->level);
            renderer.DrawText(dispEntry.pEntry->message, lineBounds, color);
        }
        y -= lineH;
    }
}

void DebugConsoleWindow::RenderInputLine(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto bgColor = palette.Get(Theme::SemanticColor::WindowBackground);
    auto borderColor = palette.Get(Theme::SemanticColor::WindowBorder);
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.inputArea.x, m_layout.inputArea.y,
                          m_layout.inputArea.x + m_layout.inputArea.width,
                          m_layout.inputArea.y + m_layout.inputArea.height };

    renderer.FillRectangle(area, { bgColor.r, bgColor.g, bgColor.b, bgColor.a }, 0.2f);
    renderer.DrawRectangle(area, { borderColor.r, borderColor.g, borderColor.b, borderColor.a }, 1.0f);

    D2D1_RECT_F textBounds = { area.left + 4.0f, area.top + 2.0f, area.right - 4.0f, area.bottom - 2.0f };
    renderer.DrawText(m_inputLine, textBounds, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void DebugConsoleWindow::RenderStatusBar(Graphics::Renderer& renderer) noexcept
{
    const auto& palette = m_pTheme->GetCurrentTheme().GetPalette();
    auto textColor = palette.Get(Theme::SemanticColor::TextPrimary);

    D2D1_RECT_F area = { m_layout.statusBarArea.x, m_layout.statusBarArea.y,
                          m_layout.statusBarArea.x + m_layout.statusBarArea.width,
                          m_layout.statusBarArea.y + m_layout.statusBarArea.height };

    size_t visibleCount = 0;
    for (const auto& de : m_displayEntries)
    {
        if (!de.filtered && de.pEntry) visibleCount++;
    }

    std::wstring status = L"Entries: " + std::to_wstring(visibleCount) +
        L" | Filter: " + (m_searchFilter.empty() ? L"none" : m_searchFilter);

    renderer.DrawText(status, area, { textColor.r, textColor.g, textColor.b, textColor.a });
}

void DebugConsoleWindow::ExecuteCommand() noexcept
{
    if (m_inputLine.empty()) return;

    m_inputLine.clear();
    m_cursorPos = 0;
}

Graphics::Color DebugConsoleWindow::GetLevelColor(LogLevel level) const noexcept
{
    switch (level)
    {
    case LogLevel::Trace:    return Graphics::Color{ 0.5f, 0.5f, 0.5f };
    case LogLevel::Debug:    return Graphics::Color{ 0.6f, 0.6f, 0.8f };
    case LogLevel::Info:     return Graphics::Color{ 0.8f, 0.8f, 0.8f };
    case LogLevel::Warning:  return Graphics::Color{ 0.9f, 0.8f, 0.2f };
    case LogLevel::Error:    return Graphics::Color{ 0.9f, 0.2f, 0.2f };
    case LogLevel::Fatal:    return Graphics::Color{ 1.0f, 0.1f, 0.1f };
    default:                 return Graphics::Color{ 0.8f, 0.8f, 0.8f };
    }
}

} // namespace DragonOS::Diagnostics
