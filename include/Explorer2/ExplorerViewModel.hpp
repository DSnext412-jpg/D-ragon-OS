// ============================================================================
//  ExplorerViewModel.hpp — MVVM model for one Explorer tab.
//
//  Part 2 (file list): owns the presentation state of a single tab (current
//  path, view mode, sort, group, hidden-file filter, quick filter) and
//  produces the unified row sequence consumed by the virtualized UIListView
//  through an ObservableCollection + CollectionViewSource.
//
//  Sorting, filtering and grouping are implemented at this collection-view
//  level so group headers, collapsed groups and search results all flow
//  through one code path.
// ============================================================================

#pragma once

#include <Explorer2/ExplorerTypes.hpp>
#include <DragonUI/DataBinding/ObservableCollection.hpp>

#include <set>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

using DragonUI::MakeObservableCollection;
using DragonUI::ObservableCollectionPtr;

class ExplorerViewModel final {
public:
    ExplorerViewModel() noexcept;

    // ── Location ─────────────────────────────────────────────────────────

    void SetCurrentPath(std::wstring path) noexcept { m_currentPath = std::move(path); }
    [[nodiscard]] const std::wstring& GetCurrentPath() const noexcept { return m_currentPath; }

    // ── Data input ───────────────────────────────────────────────────────

    /// Replaces the directory contents (from cache / load pipeline / search).
    void SetEntries(std::vector<FileSystem::FileEntry> entries) noexcept;

    /// Live incremental append used by deep search.
    void AppendEntry(FileSystem::FileEntry entry) noexcept;

    // ── Presentation state ───────────────────────────────────────────────

    void SetViewMode(ViewMode mode) noexcept;
    [[nodiscard]] ViewMode GetViewMode() const noexcept { return m_viewMode; }

    void SetSort(SortKey key, SortDir dir) noexcept;
    [[nodiscard]] SortKey GetSortKey() const noexcept { return m_sortKey; }
    [[nodiscard]] SortDir GetSortDir() const noexcept { return m_sortDir; }
    void ToggleSort(SortKey key) noexcept;

    void SetGroupMode(GroupMode mode) noexcept;
    [[nodiscard]] GroupMode GetGroupMode() const noexcept { return m_groupMode; }

    void SetShowHidden(bool show) noexcept;
    [[nodiscard]] bool GetShowHidden() const noexcept { return m_showHidden; }

    void SetQuickFilter(std::wstring text) noexcept;
    [[nodiscard]] const std::wstring& GetQuickFilter() const noexcept { return m_quickFilter; }
    [[nodiscard]] bool IsFiltered() const noexcept { return !m_quickFilter.empty(); }

    void ToggleGroupCollapsed(const std::wstring& label) noexcept;
    [[nodiscard]] bool IsGroupCollapsed(const std::wstring& label) const noexcept
    {
        return m_collapsedGroups.count(label) > 0;
    }
    void ExpandAllGroups() noexcept { m_collapsedGroups.clear(); RebuildRows(); }

    /// Search-result mode replaces normal folder semantics in the status bar.
    void SetSearchResultsMode(bool active) noexcept;
    [[nodiscard]] bool IsSearchResultsMode() const noexcept { return m_searchMode; }

    // ── Row output ───────────────────────────────────────────────────────

    /// Regenerates the row sequence from current state.
    void RebuildRows() noexcept;

    [[nodiscard]] ObservableCollectionPtr<FileRow> GetRows() const noexcept { return m_rows; }

    [[nodiscard]] size_t GetEntryCount() const noexcept;        ///< Visible entries.
    [[nodiscard]] size_t GetTotalEntryCount() const noexcept { return m_entries.size(); }
    [[nodiscard]] size_t GetHiddenByFilterCount() const noexcept;

    /// Entries currently visible (post-filter), in model order.
    [[nodiscard]] std::vector<const FileSystem::FileEntry*> GetVisibleEntries() const noexcept;

    /// Stable comparison shared by sorting and grouping order.
    [[nodiscard]] int CompareEntries(
        const FileSystem::FileEntry& a, const FileSystem::FileEntry& b,
        SortKey key) const noexcept;

private:
    [[nodiscard]] bool PassesFilters(const FileSystem::FileEntry& entry) const noexcept;
    [[nodiscard]] std::wstring GroupLabelFor(const FileSystem::FileEntry& entry) const noexcept;
    [[nodiscard]] int CompareGroupLabels(const std::wstring& a, const std::wstring& b) const noexcept;

    std::wstring m_currentPath;
    std::vector<FileSystem::FileEntry> m_entries;

    std::wstring m_quickFilter;
    ViewMode  m_viewMode{ ViewMode::Details };
    SortKey   m_sortKey{ SortKey::Name };
    SortDir   m_sortDir{ SortDir::Ascending };
    GroupMode m_groupMode{ GroupMode::None };
    bool      m_showHidden{ false };
    bool      m_searchMode{ false };

    std::set<std::wstring> m_collapsedGroups;

    ObservableCollectionPtr<FileRow> m_rows{ MakeObservableCollection<FileRow>() };
};

} // namespace DragonOS::Explorer2
