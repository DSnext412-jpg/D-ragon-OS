// ============================================================================
//  ExplorerTypes.hpp — Shared value types for the Explorer 2.0 module.
//
//  Explorer 2.0 is built exclusively on the DragonUI framework.  This header
//  defines the presentation-agnostic model types shared by every pane:
//  view modes, sorting/grouping descriptors, bookmarks, navigation history
//  and the unified row model that backs the virtualized file list.
// ============================================================================

#pragma once

#include <FileSystem/FileEntry.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace DragonOS::Explorer2 {

// ── Presentation modes ────────────────────────────────────────────────────

enum class ViewMode : uint8_t {
    Details, //< Column headers + rows (default).
    List,    //< Single-column list with icons.
    Tiles,   //< Icon grid with two-line labels.
};

enum class SortKey : uint8_t {
    Name,
    Size,
    Type,
    DateModified,
};

enum class SortDir : uint8_t {
    Ascending,
    Descending,
};

enum class GroupMode : uint8_t {
    None,
    ByType,
    ByDateModified,
    BySize,
    ByName,
};

[[nodiscard]] constexpr std::wstring_view ToString(ViewMode mode) noexcept
{
    switch (mode)
    {
    case ViewMode::Details: return L"Details";
    case ViewMode::List:    return L"List";
    case ViewMode::Tiles:   return L"Tiles";
    }
    return L"Details";
}

[[nodiscard]] constexpr std::wstring_view ToString(SortKey key) noexcept
{
    switch (key)
    {
    case SortKey::Name:          return L"Name";
    case SortKey::Size:          return L"Size";
    case SortKey::Type:          return L"Type";
    case SortKey::DateModified:  return L"Date modified";
    }
    return L"Name";
}

[[nodiscard]] constexpr std::wstring_view ToString(GroupMode mode) noexcept
{
    switch (mode)
    {
    case GroupMode::None:           return L"None";
    case GroupMode::ByType:         return L"Type";
    case GroupMode::ByDateModified: return L"Date modified";
    case GroupMode::BySize:         return L"Size";
    case GroupMode::ByName:         return L"Name";
    }
    return L"None";
}

// ── Bookmarks (Quick access) ──────────────────────────────────────────────

struct Bookmark final {
    std::wstring name;
    std::wstring path;
    uint32_t     glyph{ 0 };
};

/// In-memory bookmark store shared by every Explorer window.
class BookmarkStore final {
public:
    void Add(Bookmark bookmark) noexcept { m_bookmarks.push_back(std::move(bookmark)); }
    bool Remove(const std::wstring& path) noexcept
    {
        for (auto it = m_bookmarks.begin(); it != m_bookmarks.end(); ++it)
        {
            if (it->path == path)
            {
                m_bookmarks.erase(it);
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] bool Contains(const std::wstring& path) const noexcept
    {
        for (const auto& b : m_bookmarks)
        {
            if (b.path == path) { return true; }
        }
        return false;
    }
    [[nodiscard]] const std::vector<Bookmark>& GetAll() const noexcept { return m_bookmarks; }

private:
    std::vector<Bookmark> m_bookmarks;
};

// ── Per-tab state ─────────────────────────────────────────────────────────

struct TabState final {
    uint64_t    id{ 0 };
    std::wstring title;      ///< Displayed in the tab strip (folder name).
    std::wstring path;       ///< Current directory of the tab.
    ViewMode    viewMode{ ViewMode::Details };
};

struct ClosedTab final {
    std::wstring path;
    ViewMode     viewMode{ ViewMode::Details };
};

/// Back/forward navigation history for a single tab.
class NavHistory final {
public:
    void Reset(std::wstring path) noexcept
    {
        m_current = std::move(path);
        m_back.clear();
        m_forward.clear();
    }

    /// Moves to @p path as a new forward location.  Returns the previous location.
    [[nodiscard]] std::wstring Push(std::wstring path) noexcept
    {
        std::wstring previous = m_current;
        if (!m_current.empty() && m_current != path)
        {
            m_back.push_back(m_current);
            if (m_back.size() > kMaxEntries) { m_back.erase(m_back.begin()); }
            m_forward.clear();
        }
        m_current = std::move(path);
        return previous;
    }

    /// Steps back.  Returns empty when there is nothing to go back to.
    [[nodiscard]] std::wstring GoBack() noexcept
    {
        if (m_back.empty()) { return {}; }
        m_forward.push_back(m_current);
        std::wstring target = m_back.back();
        m_back.pop_back();
        m_current = target;
        return target;
    }

    /// Steps forward.  Returns empty when there is nothing to go forward to.
    [[nodiscard]] std::wstring GoForward() noexcept
    {
        if (m_forward.empty()) { return {}; }
        m_back.push_back(m_current);
        std::wstring target = m_forward.back();
        m_forward.pop_back();
        m_current = target;
        return target;
    }

    [[nodiscard]] const std::wstring& Current() const noexcept { return m_current; }
    [[nodiscard]] bool CanGoBack() const noexcept { return !m_back.empty(); }
    [[nodiscard]] bool CanGoForward() const noexcept { return !m_forward.empty(); }
    [[nodiscard]] const std::vector<std::wstring>& BackStack() const noexcept { return m_back; }
    [[nodiscard]] const std::vector<std::wstring>& ForwardStack() const noexcept { return m_forward; }

private:
    static constexpr size_t kMaxEntries = 64;

    std::wstring m_current;
    std::vector<std::wstring> m_back;
    std::vector<std::wstring> m_forward;
};

// ── Unified row model ─────────────────────────────────────────────────────
//
// The virtualized UIListView displays one flat sequence of rows.  When
// grouping is active, group headers and entries share that sequence so the
// control's virtualization, selection and scrolling continue to work.

struct FileRow final {
    enum class Kind : uint8_t { GroupHeader, Entry };

    Kind kind{ Kind::Entry };
    std::wstring groupLabel;              ///< Header rows only.
    size_t       groupCount{ 0 };         ///< Header rows only.
    FileSystem::FileEntry entry;          ///< Entry rows only.

    [[nodiscard]] bool IsHeader() const noexcept { return kind == Kind::GroupHeader; }
};

// ── Shared helpers ────────────────────────────────────────────────────────

/// Friendly type description ("File folder", "PNG image", ...).
[[nodiscard]] std::wstring GetTypeDisplayName(const FileSystem::FileEntry& entry) noexcept;

/// Case-insensitive comparison used across the module.
[[nodiscard]] int CompareNoCase(std::wstring_view a, std::wstring_view b) noexcept;

/// Natural (numeric-aware) ordering, e.g. "File2" < "File10".
[[nodiscard]] bool NaturalLess(const std::wstring& a, const std::wstring& b) noexcept;

/// Splits an absolute path into its path segments ("C:", "Users", "dipak").
[[nodiscard]] std::vector<std::wstring> SplitPath(const std::wstring& path) noexcept;

/// Builds the path prefix for the first @p count segments of @p path.
[[nodiscard]] std::wstring JoinPathSegments(const std::vector<std::wstring>& segments, size_t count) noexcept;

} // namespace DragonOS::Explorer2
