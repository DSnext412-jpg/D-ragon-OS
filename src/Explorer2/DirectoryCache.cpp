#include <Explorer2/DirectoryCache.hpp>

#include <algorithm>

namespace DragonOS::Explorer2 {

// ============================================================================
//  Cache queries / mutations
// ============================================================================

const FileSystem::DirectoryResult* DirectoryCache::Get(
    const std::wstring& path, double ttlSeconds) noexcept
{
    const auto it = m_entries.find(path);
    if (it == m_entries.end())
    {
        ++m_stats.misses;
        return nullptr;
    }

    const auto age = std::chrono::duration<double>(Clock::now() - it->second.loadedAt).count();
    if (age > ttlSeconds)
    {
        ++m_stats.misses;
        return nullptr;
    }

    ++m_stats.hits;
    return &it->second.result;
}

void DirectoryCache::Put(const std::wstring& path, FileSystem::DirectoryResult result) noexcept
{
    Entry entry;
    entry.result = std::move(result);
    entry.loadedAt = Clock::now();
    m_entries.insert_or_assign(path, std::move(entry));
    m_stats.size = m_entries.size();
    Trim();
}

void DirectoryCache::Invalidate(const std::wstring& path) noexcept
{
    if (m_entries.erase(path) > 0)
    {
        ++m_stats.invalidations;
    }
    m_stats.size = m_entries.size();
}

void DirectoryCache::InvalidateTree(const std::wstring& prefix) noexcept
{
    for (auto it = m_entries.begin(); it != m_entries.end();)
    {
        if (it->first.rfind(prefix, 0) == 0 || prefix.empty())
        {
            it = m_entries.erase(it);
            ++m_stats.invalidations;
        }
        else
        {
            ++it;
        }
    }
    m_stats.size = m_entries.size();
}

void DirectoryCache::Clear() noexcept
{
    m_entries.clear();
    m_stats.size = 0;
}

// ============================================================================
//  Internal
// ============================================================================

void DirectoryCache::Trim() noexcept
{
    while (m_entries.size() > MaxEntries)
    {
        // Evict the oldest entry.
        auto oldest = m_entries.begin();
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
        {
            if (it->second.loadedAt < oldest->second.loadedAt)
            {
                oldest = it;
            }
        }
        m_entries.erase(oldest);
    }
    m_stats.size = m_entries.size();
}

} // namespace DragonOS::Explorer2
