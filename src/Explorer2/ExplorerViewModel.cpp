#include <Explorer2/ExplorerViewModel.hpp>
#include <Explorer2/IconLibrary.hpp>

#include <algorithm>
#include <cwctype>

#include <windows.h>

namespace DragonOS::Explorer2 {

namespace {

using FileSystem::FileEntry;

[[nodiscard]] bool ContainsNoCase(const std::wstring& haystack, const std::wstring& needle) noexcept
{
    if (needle.empty()) { return true; }
    if (needle.size() > haystack.size()) { return false; }

    const auto it = std::search(
        haystack.begin(), haystack.end(), needle.begin(), needle.end(),
        [](wchar_t a, wchar_t b) { return std::towlower(a) == std::towlower(b); });
    return it != haystack.end();
}

[[nodiscard]] int CompareUint64(uint64_t a, uint64_t b) noexcept
{
    return a == b ? 0 : (a < b ? -1 : 1);
}

} // namespace

// ============================================================================
//  Construction
// ============================================================================

ExplorerViewModel::ExplorerViewModel() noexcept = default;

// ============================================================================
//  Data input
// ============================================================================

void ExplorerViewModel::SetEntries(std::vector<FileEntry> entries) noexcept
{
    m_entries = std::move(entries);
    RebuildRows();
}

void ExplorerViewModel::AppendEntry(FileEntry entry) noexcept
{
    m_entries.push_back(std::move(entry));

    // Cheap incremental path: only append when the row sequence is a plain,
    // ungrouped, fully-passing listing; otherwise rebuild.
    if (!m_searchMode || m_groupMode != GroupMode::None)
    {
        RebuildRows();
        return;
    }

    // Deep search keeps entries sorted by name as they arrive.
    FileRow row;
    row.kind = FileRow::Kind::Entry;
    row.entry = m_entries.back();
    m_rows->Add(std::move(row));
}

void ExplorerViewModel::SetSearchResultsMode(bool active) noexcept
{
    if (m_searchMode == active) { return; }
    m_searchMode = active;
    RebuildRows();
}

// ============================================================================
//  Presentation state
// ============================================================================

void ExplorerViewModel::SetViewMode(ViewMode mode) noexcept
{
    m_viewMode = mode;
}

void ExplorerViewModel::SetSort(SortKey key, SortDir dir) noexcept
{
    m_sortKey = key;
    m_sortDir = dir;
    RebuildRows();
}

void ExplorerViewModel::ToggleSort(SortKey key) noexcept
{
    if (m_sortKey == key)
    {
        m_sortDir = m_sortDir == SortDir::Ascending ? SortDir::Descending : SortDir::Ascending;
    }
    else
    {
        m_sortKey = key;
        m_sortDir = SortDir::Ascending;
    }
    RebuildRows();
}

void ExplorerViewModel::SetGroupMode(GroupMode mode) noexcept
{
    m_groupMode = mode;
    RebuildRows();
}

void ExplorerViewModel::SetShowHidden(bool show) noexcept
{
    m_showHidden = show;
    RebuildRows();
}

void ExplorerViewModel::SetQuickFilter(std::wstring text) noexcept
{
    m_quickFilter = std::move(text);
    RebuildRows();
}

void ExplorerViewModel::ToggleGroupCollapsed(const std::wstring& label) noexcept
{
    if (const auto it = m_collapsedGroups.find(label); it != m_collapsedGroups.end())
    {
        m_collapsedGroups.erase(it);
    }
    else
    {
        m_collapsedGroups.insert(label);
    }
    RebuildRows();
}

// ============================================================================
//  Row generation
// ============================================================================

void ExplorerViewModel::RebuildRows() noexcept
{
    std::vector<const FileEntry*> visible;
    visible.reserve(m_entries.size());
    for (const auto& entry : m_entries)
    {
        if (PassesFilters(entry))
        {
            visible.push_back(&entry);
        }
    }

    std::vector<FileRow> rows;

    if (m_groupMode == GroupMode::None)
    {
        std::stable_sort(visible.begin(), visible.end(),
            [this](const FileEntry* a, const FileEntry* b)
            { return CompareEntries(*a, *b, m_sortKey) < 0; });

        rows.reserve(visible.size());
        for (const FileEntry* entry : visible)
        {
            FileRow row;
            row.kind = FileRow::Kind::Entry;
            row.entry = *entry;
            rows.push_back(std::move(row));
        }
    }
    else
    {
        // Group → ordered buckets.
        struct Bucket final {
            std::wstring label;
            std::vector<const FileEntry*> entries;
        };
        std::vector<Bucket> buckets;

        for (const FileEntry* entry : visible)
        {
            const std::wstring label = GroupLabelFor(*entry);
            auto it = std::find_if(buckets.begin(), buckets.end(),
                [&label](const Bucket& b) { return b.label == label; });
            if (it == buckets.end())
            {
                buckets.push_back({ label, {} });
                it = buckets.end() - 1;
            }
            it->entries.push_back(entry);
        }

        std::sort(buckets.begin(), buckets.end(),
            [this](const Bucket& a, const Bucket& b)
            { return CompareGroupLabels(a.label, b.label) < 0; });

        for (Bucket& bucket : buckets)
        {
            std::sort(bucket.entries.begin(), bucket.entries.end(),
                [this](const FileEntry* a, const FileEntry* b)
                { return CompareEntries(*a, *b, m_sortKey) < 0; });

            FileRow header;
            header.kind = FileRow::Kind::GroupHeader;
            header.groupLabel = bucket.label;
            header.groupCount = bucket.entries.size();
            rows.push_back(std::move(header));

            if (m_collapsedGroups.count(bucket.label) > 0)
            {
                continue;
            }

            for (const FileEntry* entry : bucket.entries)
            {
                FileRow row;
                row.kind = FileRow::Kind::Entry;
                row.entry = *entry;
                rows.push_back(std::move(row));
            }
        }
    }

    m_rows->Assign(std::move(rows));
}

size_t ExplorerViewModel::GetEntryCount() const noexcept
{
    size_t count = 0;
    for (const auto& row : *m_rows)
    {
        if (!row.IsHeader())
        {
            ++count;
        }
    }
    return count;
}

size_t ExplorerViewModel::GetHiddenByFilterCount() const noexcept
{
    if (!IsFiltered() && !m_showHidden)
    {
        // Without a quick filter the only hidden items are hidden files when
        // "show hidden" is off — count them so the status bar can explain.
        size_t hiddenFiles = 0;
        if (!m_showHidden)
        {
            for (const auto& entry : m_entries)
            {
                if (entry.IsHidden()) { ++hiddenFiles; }
            }
        }
        return hiddenFiles;
    }

    size_t filteredOut = 0;
    for (const auto& entry : m_entries)
    {
        const bool quickPass = !IsFiltered() || ContainsNoCase(entry.name, m_quickFilter);
        const bool hiddenPass = m_showHidden || !entry.IsHidden();
        if (!quickPass || !hiddenPass)
        {
            ++filteredOut;
        }
    }
    return filteredOut;
}

std::vector<const FileEntry*> ExplorerViewModel::GetVisibleEntries() const noexcept
{
    std::vector<const FileEntry*> out;
    out.reserve(m_entries.size());
    for (const auto& entry : m_entries)
    {
        if (PassesFilters(entry))
        {
            out.push_back(&entry);
        }
    }
    std::stable_sort(out.begin(), out.end(),
        [this](const FileEntry* a, const FileEntry* b)
        { return CompareEntries(*a, *b, m_sortKey) < 0; });
    return out;
}

// ============================================================================
//  Comparison / grouping rules
// ============================================================================

int ExplorerViewModel::CompareEntries(const FileEntry& a, const FileEntry& b, SortKey key) const noexcept
{
    // Folders always sort before files within the same group level.
    if (a.IsDirectory() != b.IsDirectory())
    {
        return a.IsDirectory() ? -1 : 1;
    }

    switch (key)
    {
    case SortKey::Size:
    {
        const int cmp = CompareUint64(a.size, b.size);
        if (cmp != 0) { return cmp; }
        break; // Fall through to name for ties.
    }
    case SortKey::Type:
    {
        const int cmp = CompareNoCase(GetTypeDisplayName(a), GetTypeDisplayName(b));
        if (cmp != 0) { return cmp; }
        break;
    }
    case SortKey::DateModified:
    {
        const int cmp = CompareUint64(a.lastModified, b.lastModified);
        if (cmp != 0) { return cmp; }
        break;
    }
    case SortKey::Name:
        break;
    }

    return NaturalLess(a.name, b.name) ? -1 : NaturalLess(b.name, a.name) ? 1 : 0;
}

bool ExplorerViewModel::PassesFilters(const FileEntry& entry) const noexcept
{
    if (!m_showHidden && entry.IsHidden())
    {
        return false;
    }
    if (IsFiltered() && !ContainsNoCase(entry.name, m_quickFilter))
    {
        return false;
    }
    return true;
}

std::wstring ExplorerViewModel::GroupLabelFor(const FileEntry& entry) const noexcept
{
    switch (m_groupMode)
    {
    case GroupMode::ByType:
        return GetTypeDisplayName(entry);

    case GroupMode::ByName:
    {
        if (entry.name.empty()) { return L"#"; }
        const wchar_t c = static_cast<wchar_t>(std::towupper(entry.name.front()));
        return std::iswdigit(c) ? L"0-9" : std::wstring(1, c);
    }

    case GroupMode::ByDateModified:
    {
        // FILETIME-based buckets.  One day = 86'400'000'000'000 ticks.
        constexpr uint64_t DayTicks = 864000000000ull;

        SYSTEMTIME systemTime{};
        ::GetSystemTime(&systemTime);
        FILETIME nowFt{};
        ::SystemTimeToFileTime(&systemTime, &nowFt);
        const uint64_t nowTicks =
            (static_cast<uint64_t>(nowFt.dwHighDateTime) << 32) | nowFt.dwLowDateTime;

        if (entry.lastModified == 0)          { return L"Unknown"; }
        if (entry.lastModified > nowTicks)    { return L"Future"; }
        const uint64_t ageDays = (nowTicks - entry.lastModified) / DayTicks;
        if (ageDays < 1)                      { return L"Today"; }
        if (ageDays < 2)                      { return L"Yesterday"; }
        if (ageDays < 7)                      { return L"Previous week"; }
        if (ageDays < 31)                     { return L"Previous month"; }
        if (ageDays < 365)                    { return L"Previous year"; }
        return L"A long time ago";
    }

    case GroupMode::BySize:
    {
        if (entry.IsDirectory())              { return L"Folders"; }
        constexpr uint64_t KB = 1024ull;
        constexpr uint64_t MB = KB * 1024ull;
        constexpr uint64_t GB = MB * 1024ull;
        if (entry.size == 0)                  { return L"Empty"; }
        if (entry.size < 16 * KB)             { return L"Tiny (0 - 16 KB)"; }
        if (entry.size < MB)                  { return L"Small (16 KB - 1 MB)"; }
        if (entry.size < 128 * MB)            { return L"Medium (1 - 128 MB)"; }
        if (entry.size < GB)                  { return L"Large (128 MB - 1 GB)"; }
        return L"Huge (> 1 GB)";
    }

    case GroupMode::None:
        break;
    }
    return {};
}

int ExplorerViewModel::CompareGroupLabels(const std::wstring& a, const std::wstring& b) const noexcept
{
    // Date groups have a natural chronological order worth preserving.
    if (m_groupMode == GroupMode::ByDateModified)
    {
        static constexpr std::wstring_view kOrder[] = {
            L"Unknown", L"A long time ago", L"Previous year", L"Previous month",
            L"Previous week", L"Yesterday", L"Today", L"Future",
        };
        auto rank = [](const std::wstring& s) -> int {
            for (size_t i = 0; i < std::size(kOrder); ++i)
            {
                if (kOrder[i] == s) { return static_cast<int>(i); }
            }
            return INT_MAX;
        };
        const int ra = rank(a);
        const int rb = rank(b);
        if (ra != rb) { return ra < rb ? -1 : 1; }
        return 0;
    }
    return CompareNoCase(a, b);
}

} // namespace DragonOS::Explorer2
