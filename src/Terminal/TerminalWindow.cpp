#include <Terminal/TerminalWindow.hpp>
#include <Terminal/TerminalTheme.hpp>

#include <UI/Core/UIRenderer.hpp>

#include <Command/Builtins.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Graphics/Renderer.hpp>
#include <Input/HitTest.hpp>
#include <Input/InputManager.hpp>
#include <Input/Keyboard.hpp>
#include <Input/KeyCodes.hpp>
#include <Input/MouseManager.hpp>
#include <Theme/ThemeManager.hpp>
#include <Theme/ThemeMetrics.hpp>
#include <WindowManager/DragonWindow.hpp>

#include <Windows.h>
#undef GetCurrentDirectory
#undef SetCurrentDirectory
#include <d2d1.h>
#include <dwrite.h>

#include <algorithm>
#include <cmath>
#include <cwchar>

namespace DragonOS::Terminal {

namespace {

// ── HistoryCommand (registered per-terminal) ────────────────────────────

class HistoryCommand final : public ::DragonOS::Command::Command {
public:
    explicit HistoryCommand(History::History& history) noexcept
        : m_history{ &history }
    {
    }

    [[nodiscard]] const std::wstring& GetName() const noexcept override
    {
        static const std::wstring name = L"history";
        return name;
    }

    [[nodiscard]] const std::wstring& GetDescription() const noexcept override
    {
        static const std::wstring desc = L"Display command history.";
        return desc;
    }

    [[nodiscard]] const std::wstring& GetHelp() const noexcept override
    {
        static const std::wstring help = L"history\n  Show the list of previously executed commands.";
        return help;
    }

    ::DragonOS::Command::CommandResult Execute(::DragonOS::Command::CommandContext&) override
    {
        ::DragonOS::Command::CommandResult result;
        const auto& entries = m_history->GetAll();

        for (size_t i = 0; i < entries.size(); ++i)
        {
            wchar_t lineNum[32];
            ::swprintf_s(lineNum, L"%4zu", i + 1);
            result.output += lineNum;
            result.output += L"  ";
            result.output += entries[i];
            result.output += L"\n";
        }

        return result;
    }

private:
    History::History* m_history{ nullptr };
};

} // anonymous namespace

// ============================================================================
//  Construction / Destruction
// ============================================================================

TerminalWindow::TerminalWindow() noexcept
    : m_buffer{ 2000 }
{
    Command::RegisterBuiltins(m_commandRegistry);

    m_commandRegistry.Register(
        std::make_unique<HistoryCommand>(m_history));
}

TerminalWindow::~TerminalWindow() noexcept
{
    if (m_pTextFormat) { m_pTextFormat->Release(); }
}

// ============================================================================
//  Initialization
// ============================================================================

void TerminalWindow::SetDependencies(
    WindowManager::DragonWindow&  window,
    Theme::ThemeManager&          themeManager,
    Input::InputManager&          inputManager,
    Input::MouseManager&          mouseManager,
    FileSystem::FileSystemService& fsService) noexcept
{
    m_pWindow = &window;
    m_pTheme  = &themeManager;
    m_pInput  = &inputManager;
    m_pMouse  = &mouseManager;
    m_pFS     = &fsService;

    m_colors = TerminalColors::FromTheme(themeManager);

    auto drives = m_pFS->GetLogicalDrives();
    if (!drives.empty())
    {
        m_currentDirectory = drives[0] + L"\\";
    }
    else
    {
        m_currentDirectory = L"C:\\";
    }

    PrintPrompt();

    InitializeDirectWrite();

    m_uiStatusBar = std::make_unique<UI::StatusBar>();

    m_initialized = true;
}

UI::UIRenderer TerminalWindow::MakeUIRenderer(Graphics::Renderer& renderer) const noexcept
{
    return UI::UIRenderer(renderer, *m_pTheme);
}

void TerminalWindow::InitializeDirectWrite() noexcept
{
    if (m_pTextFormat)
    {
        m_pTextFormat->Release();
        m_pTextFormat = nullptr;
    }

    HRESULT hr = ::DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

    if (FAILED(hr)) { return; }

    const float fontSize = 14.0f;

    hr = m_pDWriteFactory->CreateTextFormat(
        L"Cascadia Code",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"en-US",
        &m_pTextFormat);

    if (FAILED(hr) || !m_pTextFormat)
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Consolas",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-US",
            &m_pTextFormat);
    }

    if (FAILED(hr) || !m_pTextFormat)
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Courier New",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-US",
            &m_pTextFormat);
    }

    if (m_pTextFormat)
    {
        m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        m_charHeight = fontSize * 1.4f;

        IDWriteTextLayout* pLayout = nullptr;
        m_pDWriteFactory->CreateTextLayout(
            L"A", 1, m_pTextFormat, 1000.0f, 1000.0f, &pLayout);

        if (pLayout)
        {
            DWRITE_TEXT_METRICS metrics;
            pLayout->GetMetrics(&metrics);
            m_charWidth = metrics.widthIncludingTrailingWhitespace;
            pLayout->Release();
        }

        if (m_charWidth < 1.0f)
        {
            m_charWidth = fontSize * 0.6f;
        }
    }
}

// ============================================================================
//  Layout
// ============================================================================

void TerminalWindow::RecalculateLayout() noexcept
{
    if (!m_pWindow) { return; }

    const float w = m_pWindow->GetWidth();
    const float h = m_pWindow->GetHeight();

    m_viewportWidth  = w;
    m_viewportHeight = h;

    const float pad = m_layout.padding;

    m_layout.outputArea = Input::Bounds{
        pad,
        pad,
        w - m_layout.scrollbarWidth - pad * 2,
        h - m_layout.statusLineHeight - m_charHeight - pad * 4
    };

    m_layout.inputArea = Input::Bounds{
        pad,
        h - m_layout.statusLineHeight - m_charHeight - pad * 2,
        w - pad * 2,
        m_charHeight + pad
    };

    m_layout.statusArea = Input::Bounds{
        pad,
        h - m_layout.statusLineHeight - pad,
        w - pad * 2,
        m_layout.statusLineHeight
    };

    if (m_uiStatusBar)
    {
        D2D1_RECT_F sbBounds = {
            m_layout.statusArea.x, m_layout.statusArea.y,
            m_layout.statusArea.Right(), m_layout.statusArea.Bottom()
        };
        m_uiStatusBar->Measure(D2D1::RectF(0, 0, m_layout.statusArea.width, m_layout.statusArea.height));
        m_uiStatusBar->Arrange(sbBounds);
    }

    m_visibleLines = static_cast<size_t>(
        m_layout.outputArea.height / m_charHeight);
    if (m_visibleLines < 1) { m_visibleLines = 1; }

    m_visibleCols = static_cast<size_t>(
        m_layout.outputArea.width / m_charWidth);
    if (m_visibleCols < 10) { m_visibleCols = 10; }
}

// ============================================================================
//  Per-frame
// ============================================================================

void TerminalWindow::Update() noexcept
{
    if (!m_initialized) { return; }

    {
        const float animSpeed = 4.0f;
        if (m_scrollbarHovered)
        {
            m_scrollbarHoverAnim += 0.016f * animSpeed;
            if (m_scrollbarHoverAnim > 1.0f) { m_scrollbarHoverAnim = 1.0f; }
        }
        else
        {
            m_scrollbarHoverAnim -= 0.016f * animSpeed;
            if (m_scrollbarHoverAnim < 0.0f) { m_scrollbarHoverAnim = 0.0f; }
        }
    }
}

void TerminalWindow::Render(Graphics::Renderer& renderer) noexcept
{
    if (!m_initialized || !m_pWindow || !m_pWindow->IsVisible()) { return; }

    RecalculateLayout();

    auto* target = renderer.GetRenderTarget();
    if (!target) { return; }

    const auto bounds = m_pWindow->GetBounds();
    const float wx = bounds.x;
    const float wy = bounds.y;

    D2D1_MATRIX_3X2_F originalTransform;
    target->GetTransform(&originalTransform);

    target->SetTransform(D2D1::Matrix3x2F::Translation(wx, wy));

    RenderOutput(renderer);
    RenderInputLine(renderer);
    RenderStatusLine(renderer);
    RenderScrollbar(renderer);

    target->SetTransform(originalTransform);
}

void TerminalWindow::RenderOutput(Graphics::Renderer& renderer) noexcept
{
    auto* target = renderer.GetRenderTarget();
    if (!target) { return; }

    const auto& outBounds = m_layout.outputArea;

    auto* bgBrush = renderer.GetBrush(m_colors.background);
    if (bgBrush)
    {
        D2D1_RECT_F bg = D2D1::RectF(
            outBounds.x, outBounds.y,
            outBounds.x + outBounds.width,
            outBounds.y + outBounds.height);
        target->FillRectangle(&bg, bgBrush);
    }

    if (m_scrollAtBottom)
    {
        m_buffer.SetScrollOffset(0);
    }

    auto range = m_buffer.GetVisibleRange(
        m_buffer.GetScrollOffset(), m_visibleLines);

    if (range.count == 0 || !m_pTextFormat) { return; }

    auto* textBrush = renderer.GetBrush(m_colors.foreground);
    if (!textBrush) { return; }

    for (size_t i = 0; i < range.count; ++i)
    {
        const size_t lineIdx = range.start + i;
        const auto& line = m_buffer.GetLine(lineIdx);

        if (line.empty()) { continue; }

        std::wstring displayLine = line;
        bool exitTerm = false;

        if (displayLine.size() >= 5 && displayLine[0] == L'\x1b')
        {
            if (displayLine == L"\x1b[2J")
            {
                m_buffer.Clear();
                PrintPrompt();
                return;
            }
            if (displayLine == L"\x1b[EXIT")
            {
                exitTerm = true;
                displayLine.clear();
            }
            else
            {
                displayLine = line;
            }
        }

        if (exitTerm)
        {
            m_pWindow->Close();
            return;
        }

        const D2D1_RECT_F textRect = D2D1::RectF(
            outBounds.x + 2,
            outBounds.y + static_cast<float>(i) * m_charHeight,
            outBounds.x + outBounds.width - 2,
            outBounds.y + static_cast<float>(i + 1) * m_charHeight);

        target->DrawText(
            displayLine.c_str(),
            static_cast<UINT32>(displayLine.size()),
            m_pTextFormat,
            &textRect,
            textBrush);
    }
}

void TerminalWindow::RenderInputLine(Graphics::Renderer& renderer) noexcept
{
    auto* target = renderer.GetRenderTarget();
    if (!target || !m_pTextFormat) { return; }

    const auto& inBounds = m_layout.inputArea;

    auto* textBrush = renderer.GetBrush(m_colors.foreground);
    if (!textBrush) { return; }

    if (!m_inputLine.empty())
    {
        const D2D1_RECT_F textRect = D2D1::RectF(
            inBounds.x,
            inBounds.y,
            inBounds.x + inBounds.width,
            inBounds.y + inBounds.height);

        target->DrawText(
            m_inputLine.c_str(),
            static_cast<UINT32>(m_inputLine.size()),
            m_pTextFormat,
            &textRect,
            textBrush);
    }

    float cursorX = inBounds.x;
    if (m_cursorPos > 0)
    {
        std::wstring before = m_inputLine.substr(0, m_cursorPos);
        IDWriteTextLayout* pLayout = nullptr;
        m_pDWriteFactory->CreateTextLayout(
            before.c_str(),
            static_cast<UINT32>(before.size()),
            m_pTextFormat,
            1000.0f,
            1000.0f,
            &pLayout);
        if (pLayout)
        {
            DWRITE_TEXT_METRICS tm;
            pLayout->GetMetrics(&tm);
            cursorX += tm.widthIncludingTrailingWhitespace;
            pLayout->Release();
        }
    }

    const bool cursorVisible = (static_cast<int>(::GetTickCount64() / 500) % 2 == 0);
    if (cursorVisible)
    {
        auto* cursorBrush = renderer.GetBrush(m_colors.cursor);
        if (cursorBrush)
        {
            D2D1_RECT_F cursorRect = D2D1::RectF(
                cursorX,
                inBounds.y + 1,
                cursorX + 2.0f,
                inBounds.y + m_charHeight - 1);
            target->FillRectangle(&cursorRect, cursorBrush);
        }
    }
}

void TerminalWindow::RenderStatusLine(Graphics::Renderer& renderer) noexcept
{
    if (!m_uiStatusBar) return;

    wchar_t statusBuf[128];
    ::swprintf_s(statusBuf,
        L"Lines: %zu  |  CWD: %s",
        m_buffer.GetLineCount(),
        m_currentDirectory.c_str());

    m_uiStatusBar->SetText(statusBuf);

    auto uiRenderer = MakeUIRenderer(renderer);
    m_uiStatusBar->Render(uiRenderer);
}

void TerminalWindow::RenderScrollbar(Graphics::Renderer& renderer) noexcept
{
    auto* target = renderer.GetRenderTarget();
    if (!target) { return; }

    if (m_buffer.GetLineCount() <= m_visibleLines) { return; }

    const auto& outBounds = m_layout.outputArea;
    const float sbX = outBounds.x + outBounds.width;
    const float sbW = m_layout.scrollbarWidth;

    float alpha = 0.3f + m_scrollbarHoverAnim * 0.4f;
    if (alpha > 1.0f) { alpha = 1.0f; }

    auto trackColor = m_colors.scrollbar;
    trackColor.a *= alpha;

    auto* trackBrush = renderer.GetBrush(trackColor);
    if (trackBrush)
    {
        D2D1_RECT_F track = D2D1::RectF(
            sbX, outBounds.y, sbX + sbW, outBounds.y + outBounds.height);
        target->FillRectangle(&track, trackBrush);
    }

    auto* thumbBrush = renderer.GetBrush(m_colors.scrollbar);
    if (thumbBrush)
    {
        float totalLines = static_cast<float>(m_buffer.GetLineCount());
        float thumbHeight = (static_cast<float>(m_visibleLines) / totalLines) * outBounds.height;
        if (thumbHeight < 16.0f) { thumbHeight = 16.0f; }

        float scrollFrac = 0.0f;
        size_t maxScroll = m_buffer.GetMaxScrollOffset();
        if (maxScroll > 0)
        {
            scrollFrac = static_cast<float>(m_buffer.GetScrollOffset()) /
                static_cast<float>(maxScroll);
        }
        if (scrollFrac > 1.0f) { scrollFrac = 1.0f; }

        float thumbY = outBounds.y + scrollFrac * (outBounds.height - thumbHeight);

        D2D1_RECT_F thumb = D2D1::RectF(
            sbX, thumbY, sbX + sbW, thumbY + thumbHeight);
        target->FillRectangle(&thumb, thumbBrush);
    }
}

// ============================================================================
//  Keyboard Input
// ============================================================================

void TerminalWindow::ProcessKeyboardInput(
    Input::InputManager& inputManager) noexcept
{
    if (!m_initialized || !m_pWindow) { return; }

    auto& keyboard = inputManager.GetKeyboard();

    for (wchar_t ch : inputManager.GetCharBuffer())
    {
        if (ch == L'\r')
        {
            ExecuteCommand();
        }
        else if (ch == L'\b')
        {
            DeleteChar();
        }
        else if (ch == L'\t')
        {
        }
        else if (ch >= 32)
        {
            InsertChar(ch);
        }
    }

    using Key = Input::KeyCode;

    if (keyboard.IsKeyReleased(Key::Up))
    {
        const auto& prev = m_history.GetPrevious();
        if (!prev.empty())
        {
            m_inputLine = prev;
            m_cursorPos = m_inputLine.size();
        }
    }
    else if (keyboard.IsKeyReleased(Key::Down))
    {
        const auto& next = m_history.GetNext();
        if (!next.empty())
        {
            m_inputLine = next;
            m_cursorPos = m_inputLine.size();
        }
        else
        {
            m_inputLine.clear();
            m_cursorPos = 0;
        }
    }
    else if (keyboard.IsKeyReleased(Key::Left))
    {
        MoveCursorLeft();
    }
    else if (keyboard.IsKeyReleased(Key::Right))
    {
        MoveCursorRight();
    }
    else if (keyboard.IsKeyReleased(Key::Home))
    {
        MoveCursorHome();
    }
    else if (keyboard.IsKeyReleased(Key::End))
    {
        MoveCursorEnd();
    }
    else if (keyboard.IsKeyReleased(Key::Delete))
    {
        if (m_cursorPos < m_inputLine.size())
        {
            m_inputLine.erase(m_cursorPos, 1);
        }
    }
    else if (keyboard.IsKeyReleased(Key::PageUp))
    {
        ScrollUp();
    }
    else if (keyboard.IsKeyReleased(Key::PageDown))
    {
        ScrollDown();
    }
    else if (keyboard.IsKeyReleased(Key::Escape))
    {
        m_inputLine.clear();
        m_cursorPos = 0;
    }
}

// ============================================================================
//  Output helpers
// ============================================================================

void TerminalWindow::WriteOutput(const std::wstring& text) noexcept
{
    m_buffer.Append(text);
    if (m_scrollAtBottom)
    {
        m_buffer.SetScrollOffset(0);
    }
}

void TerminalWindow::WriteLine(const std::wstring& text) noexcept
{
    WriteOutput(text + L"\n");
}

void TerminalWindow::PrintPrompt() noexcept
{
    m_promptString = m_currentDirectory + L"> ";
    WriteOutput(m_promptString);
}

// ============================================================================
//  Command execution
// ============================================================================

void TerminalWindow::ExecuteCommand() noexcept
{
    std::wstring cmdLine = m_inputLine;
    m_inputLine.clear();
    m_cursorPos = 0;

    if (!cmdLine.empty())
    {
        m_history.Add(cmdLine);
        WriteOutput(cmdLine + L"\n");
    }
    else
    {
        PrintPrompt();
        return;
    }

    ::DragonOS::Command::CommandContext context{
        *m_pFS,
        m_currentDirectory,
        {}
    };

    auto result = m_commandRegistry.Execute(cmdLine, context);

    m_currentDirectory = context.GetCurrentDirectory();

    if (!result.output.empty())
    {
        WriteOutput(result.output);
        if (result.output.back() != L'\n')
        {
            WriteOutput(L"\n");
        }
    }

    if (!result.error.empty())
    {
        WriteOutput(result.error);
        if (result.error.back() != L'\n')
        {
            WriteOutput(L"\n");
        }
    }

    if (result.status == Command::CommandStatus::NotFound)
    {
        WriteOutput(L"'" + cmdLine + L"' is not recognized as a command.\n");
    }

    PrintPrompt();
}

// ============================================================================
//  Input editing helpers
// ============================================================================

void TerminalWindow::InsertChar(wchar_t ch) noexcept
{
    if (m_inputLine.size() >= 512) { return; }
    m_inputLine.insert(m_cursorPos, 1, ch);
    ++m_cursorPos;
}

void TerminalWindow::DeleteChar() noexcept
{
    if (m_cursorPos > 0 && !m_inputLine.empty())
    {
        m_inputLine.erase(m_cursorPos - 1, 1);
        --m_cursorPos;
    }
}

void TerminalWindow::MoveCursorLeft() noexcept
{
    if (m_cursorPos > 0) { --m_cursorPos; }
}

void TerminalWindow::MoveCursorRight() noexcept
{
    if (m_cursorPos < m_inputLine.size()) { ++m_cursorPos; }
}

void TerminalWindow::MoveCursorHome() noexcept
{
    m_cursorPos = 0;
}

void TerminalWindow::MoveCursorEnd() noexcept
{
    m_cursorPos = m_inputLine.size();
}

void TerminalWindow::ScrollUp() noexcept
{
    size_t maxScroll = m_buffer.GetMaxScrollOffset();
    if (m_buffer.GetScrollOffset() < maxScroll)
    {
        m_buffer.SetScrollOffset(m_buffer.GetScrollOffset() + 1);
        m_scrollAtBottom = false;
    }
}

void TerminalWindow::ScrollDown() noexcept
{
    if (m_buffer.GetScrollOffset() > 0)
    {
        m_buffer.SetScrollOffset(m_buffer.GetScrollOffset() - 1);
        m_scrollAtBottom = (m_buffer.GetScrollOffset() == 0);
    }
}

void TerminalWindow::ScrollToBottom() noexcept
{
    m_buffer.SetScrollOffset(0);
    m_scrollAtBottom = true;
}

Input::Bounds TerminalWindow::GetOutputBounds() const noexcept
{
    return m_layout.outputArea;
}

Input::Bounds TerminalWindow::GetInputBounds() const noexcept
{
    return m_layout.inputArea;
}

Input::Bounds TerminalWindow::GetStatusBounds() const noexcept
{
    return m_layout.statusArea;
}

uint64_t TerminalWindow::GetWindowId() const noexcept
{
    return m_pWindow ? m_pWindow->GetId() : 0;
}

} // namespace DragonOS::Terminal
