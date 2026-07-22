#include <History/History.hpp>

#include <algorithm>

namespace DragonOS::History {

void History::Add(const std::wstring& entry) noexcept
{
    if (entry.empty()) { return; }
    if (!m_entries.empty() && m_entries.back() == entry) { return; }

    if (m_entries.size() >= m_maxEntries)
    {
        m_entries.erase(m_entries.begin());
    }

    m_entries.push_back(entry);
    m_currentIndex = m_entries.size();
}

void History::Clear() noexcept
{
    m_entries.clear();
    m_currentIndex = 0;
}

const std::wstring& History::GetPrevious() noexcept
{
    static const std::wstring empty;

    if (m_entries.empty()) { return empty; }

    if (m_currentIndex > 0)
    {
        --m_currentIndex;
    }

    return m_entries[m_currentIndex];
}

const std::wstring& History::GetNext() noexcept
{
    static const std::wstring empty;

    if (m_entries.empty()) { return empty; }

    if (m_currentIndex < m_entries.size() - 1)
    {
        ++m_currentIndex;
        return m_entries[m_currentIndex];
    }

    m_currentIndex = m_entries.size();
    return empty;
}

const std::wstring& History::Get(size_t index) const noexcept
{
    static const std::wstring empty;
    return index < m_entries.size() ? m_entries[index] : empty;
}

} // namespace DragonOS::History
