// ============================================================================
//  DirectoryCache.hpp — LRU cache of directory listings.
//
//  Part 10 (performance): repeated navigation between folders must not
//  re-enumerate the file system every time.  The cache stores the last
//  DirectoryResult per path with a freshness TTL and an upper bound on
//  entries.  It is shared by all Explorer windows of a session through
//  ExplorerSystem, so navigating to the same folder in another window or
//  tab is instant.
// ============================================================================

#pragma once

#include <FileSystem/FileSystemService.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace DragonOS::Explorer2 {

class DirectoryCache final {
public:
    using Clock = std::chrono::steady_clock;

    struct Stats final {
        uint64_t hits{ 0 };
        uint64_t misses{ 0 };
        uint64_t invalidations{ 0 };
        size_t   size{ 0 };
    };

    /// Returns the cached listing when present and younger than @p ttlSeconds.
    [[nodiscard]] const FileSystem::DirectoryResult* Get(
        const std::wstring& path, double ttlSeconds = DefaultTtlSeconds) noexcept;

    void Put(const std::wstring& path, FileSystem::DirectoryResult result) noexcept;

    void Invalidate(const std::wstring& path) noexcept;

    /// Invalidates @p path and everything below it (used after deletes/moves).
    void InvalidateTree(const std::wstring& prefix) noexcept;

    void Clear() noexcept;

    [[nodiscard]] const Stats& GetStats() const noexcept { return m_stats; }

    static constexpr double DefaultTtlSeconds = 5.0;
    static constexpr size_t MaxEntries = 96;

private:
    struct Entry final {
        FileSystem::DirectoryResult result;
        Clock::time_point loadedAt{};
    };

    void Trim() noexcept;

    std::unordered_map<std::wstring, Entry> m_entries;
    Stats m_stats;
};

} // namespace DragonOS::Explorer2
