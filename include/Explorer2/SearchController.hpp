// ============================================================================
//  SearchController.hpp — Incremental + indexed search (Part 5).
//
//  Three layers:
//    1. Quick filter   — instant, name-based filtering of the current listing
//                        (delegated to ExplorerViewModel::SetQuickFilter).
//    2. Deep search    — recursive walk of the current folder tree with a
//                        per-frame time budget (~2 ms) so large trees never
//                        stall a frame.  Results stream into the view model.
//    3. Indexed search — optional ISearchIndexProvider hook (plugins / the
//                        OS search service); results are merged and de-duped.
//
//  All file system access is cache-first (DirectoryCache) and the walker's
//  budget keeps it responsive.  Accessibility is handled by the host's
//  AccessibilityManager (Part 11).
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <FileSystem/FileEntry.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace DragonOS::FileSystem { class FileSystemService; }
namespace DragonOS::Explorer2 { class DirectoryCache; }

namespace DragonOS::Explorer2 {

/// Hook for an external index (OS search service, plugin indexer, ...).
class ISearchIndexProvider {
public:
    virtual ~ISearchIndexProvider() = default;
    /// @return up to @p maxResults entries matching @p text.
    [[nodiscard]] virtual std::vector<FileSystem::FileEntry> Query(
        std::wstring_view text, size_t maxResults) noexcept = 0;
};

class SearchController final {
public:
    using ResultsChangedFn = std::function<void()>;
    using StatusChangedFn = std::function<void(const std::wstring&)>;

    SearchController(
        FileSystem::FileSystemService& fileSystem,
        DirectoryCache& cache) noexcept;

    // ── Wiring ───────────────────────────────────────────────────────────

    void SetResultsChanged(ResultsChangedFn cb) noexcept { m_onResultsChanged = std::move(cb); }
    void SetStatusChanged(StatusChangedFn cb) noexcept { m_onStatusChanged = std::move(cb); }

    void SetIndexProvider(ISearchIndexProvider* provider) noexcept
    {
        m_indexProvider = provider;
    }

    // ── Queries ──────────────────────────────────────────────────────────

    /// Entry point from the ribbon search box.  Empty text cancels everything.
    void SetQuery(const std::wstring& rootPath, const std::wstring& text) noexcept;

    [[nodiscard]] const std::wstring& GetQuery() const noexcept { return m_query; }
    [[nodiscard]] bool IsDeepSearchActive() const noexcept { return m_walker.active; }
    [[nodiscard]] bool HasQuery() const noexcept { return !m_query.empty(); }

    /// Frame tick; advances the time-sliced deep walker.
    void Update() noexcept;

    void CancelDeepSearch() noexcept;

    [[nodiscard]] size_t GetMatchCount() const noexcept { return m_matches.size(); }

    /// Current match set (valid until the next query/cancel).
    [[nodiscard]] const std::vector<FileSystem::FileEntry>& GetMatches() const noexcept
    {
        return m_matches;
    }

private:
    struct PendingDir final {
        std::wstring path;
        int depth{};
    };

    struct Walker final {
        bool active{ false };
        uint64_t generation{ 0 };
        std::wstring query;
        std::vector<PendingDir> pendingDirs;
        size_t dirsScanned{ 0 };
    };

    static constexpr int kMaxDepth = 8;
    static constexpr double kFrameBudgetMs = 2.0;
    static constexpr size_t kMaxMatches = 2000;

    void BeginDeepSearch(const std::wstring& rootPath, const std::wstring& query) noexcept;
    void ApplyIndexedResults() noexcept;
    void PushStatusIfDue() noexcept;

    FileSystem::FileSystemService& m_fileSystem;
    DirectoryCache& m_cache;

    std::wstring m_query;
    std::wstring m_rootPath;
    std::vector<FileSystem::FileEntry> m_matches;

    Walker m_walker;
    ISearchIndexProvider* m_indexProvider{ nullptr };

    ResultsChangedFn m_onResultsChanged;
    StatusChangedFn m_onStatusChanged;

    // Accessibility is handled by the host's AccessibilityManager
};
} // namespace DragonOS::Explorer2