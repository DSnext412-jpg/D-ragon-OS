#include <Terminal/TerminalBuffer.hpp>

#include <algorithm>

namespace DragonOS::Terminal {

TerminalBuffer::TerminalBuffer(size_t maxLines) noexcept
    : m_maxLines{ maxLines }
{
}

void TerminalBuffer::Append(const std::wstring& text) noexcept
{
    if (m_lines.size() >= m_maxLines)
    {
        m_lines.pop_front();
    }

    m_lines.push_back(text);
    m_dirty = true;
}

void TerminalBuffer::AppendLine(const std::wstring& text) noexcept
{
    Append(text + L"\n");
}

void TerminalBuffer::Clear() noexcept
{
    m_lines.clear();
    m_scrollOffset = 0;
    m_dirty = true;
}

const std::wstring& TerminalBuffer::GetLine(size_t index) const noexcept
{
    static const std::wstring empty;
    return index < m_lines.size() ? m_lines[index] : empty;
}

std::wstring TerminalBuffer::GetVisibleText(
    size_t startLine,
    size_t lineCount) const noexcept
{
    std::wstring result;
    const size_t end = (std::min)(startLine + lineCount, m_lines.size());

    for (size_t i = startLine; i < end; ++i)
    {
        result += m_lines[i];
    }

    return result;
}

TerminalBuffer::LineRange TerminalBuffer::GetVisibleRange(
    size_t scrollOffset,
    size_t viewHeight) const noexcept
{
    LineRange range;

    if (m_lines.empty())
    {
        return range;
    }

    size_t totalLines = m_lines.size();
    range.start = (std::min)(scrollOffset, totalLines > viewHeight ? totalLines - viewHeight : size_t(0));
    range.count = (std::min)(viewHeight, totalLines - range.start);

    return range;
}

size_t TerminalBuffer::GetMaxScrollOffset() const noexcept
{
    return m_lines.empty() ? 0 :
        (m_lines.size() > 10 ? m_lines.size() - 10 : 0);
}

} // namespace DragonOS::Terminal
