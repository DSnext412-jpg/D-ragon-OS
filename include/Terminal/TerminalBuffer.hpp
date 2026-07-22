#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace DragonOS::Terminal {

class TerminalBuffer final {
public:
    explicit TerminalBuffer(size_t maxLines = 2000) noexcept;

    void Append(const std::wstring& text) noexcept;
    void AppendLine(const std::wstring& text) noexcept;
    void Clear() noexcept;

    [[nodiscard]] const std::wstring& GetLine(size_t index) const noexcept;
    [[nodiscard]] size_t GetLineCount() const noexcept { return m_lines.size(); }
    [[nodiscard]] size_t GetMaxLines() const noexcept { return m_maxLines; }

    [[nodiscard]] std::wstring GetVisibleText(
        size_t startLine,
        size_t lineCount) const noexcept;

    [[nodiscard]] bool IsDirty() const noexcept { return m_dirty; }
    void ClearDirty() noexcept { m_dirty = false; }

    struct LineRange final {
        size_t start{ 0 };
        size_t count{ 0 };
    };

    [[nodiscard]] LineRange GetVisibleRange(size_t scrollOffset, size_t viewHeight) const noexcept;

    void SetScrollOffset(size_t offset) noexcept { m_scrollOffset = offset; }
    [[nodiscard]] size_t GetScrollOffset() const noexcept { return m_scrollOffset; }
    [[nodiscard]] size_t GetMaxScrollOffset() const noexcept;

private:
    std::deque<std::wstring> m_lines;
    size_t                   m_maxLines{ 2000 };
    size_t                   m_scrollOffset{ 0 };
    bool                     m_dirty{ false };
};

} // namespace DragonOS::Terminal
