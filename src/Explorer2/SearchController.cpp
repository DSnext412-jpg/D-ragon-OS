#include <Explorer2/SearchController.hpp>
#include <FileSystem/FileSystemService.hpp>
#include <Explorer2/DirectoryCache.hpp>

namespace DragonOS::Explorer2 {

using namespace DragonOS::DragonUI;

// ============================================================================
//  Construction
// ============================================================================

SearchController::SearchController(
    FileSystem::FileSystemService& fileSystem,
    DirectoryCache& cache) noexcept
    : m_fileSystem(fileSystem)
    , m_cache(cache)
{
}

// ============================================================================
//  Query management
// ============================================================================

void SearchController::SetQuery(const std::wstring& rootPath, const std::wstring& text) noexcept
{
    m_rootPath = rootPath;
    m_query = text;

    if (text.empty())
    {
        // Cancel everything - clear matches, stop walker
        m_walker.active = false;
        m_matches.clear();
        if (m_onResultsChanged) m_onResultsChanged();
        return;
    }

    // Begin deep search with indexed results merged
    BeginDeepSearch(rootPath, text);
}

[[nodiscard]] const std::wstring& SearchController::GetQuery() const noexcept
{
    return m_query;
}

bool SearchController::IsDeepSearchActive() const noexcept
{
    return m_walker.active;
}

bool SearchController::HasQuery() const noexcept
{
    return !m_query.empty();
}

// ============================================================================
//  Deep search walker (time-sliced)
// ============================================================================

void SearchController::BeginDeepSearch(const std::wstring& rootPath, const std::wstring& query) noexcept
{
    m_walker.active = true;
    m_walker.query = query;
    m_walker.generation = 1;
    m_walker.pendingDirs.clear();
    m_walker.dirsScanned = 0;
    m_matches.clear();

    // Add root directory to pending
    m_walker.pendingDirs.push_back({ rootPath, 0 });

    // Apply indexed results if provider available
    if (m_indexProvider)
    {
        ApplyIndexedResults();
    }

    // Walk the first directory in this frame
    if (!m_walker.pendingDirs.empty())
    {
        // Status updated by host window per frame
    }
}

void SearchController::ApplyIndexedResults() noexcept
{
    if (!m_indexProvider || m_rootPath.empty()) { return; }

    auto indexed = m_indexProvider->Query(m_query, kMaxMatches);
    for (const auto& entry : indexed)
    {
        // Filter by root path prefix
        if (entry.fullPath.find(m_rootPath) == 0)
        {
            // De-duplicate by full path
            bool already = false;
            for (const auto& existing : m_matches)
            {
                if (existing.fullPath == entry.fullPath)
                {
                    already = true;
                    break;
                }
            }
            if (!already && static_cast<size_t>(m_matches.size()) < kMaxMatches)
            {
                m_matches.push_back(entry);
            }
        }
    }

    if (m_onResultsChanged) m_onResultsChanged();
}

// ============================================================================
//  Frame tick - advances the time-sliced deep walker
// ============================================================================

void SearchController::Update() noexcept
{
    if (!m_walker.active) { return; }

    // Time-slice: spend at most kFrameBudgetMs per frame
    auto start = std::chrono::steady_clock::now();
    bool moreWork = false;

    do
    {
        if (m_walker.pendingDirs.empty())
        {
            m_walker.active = false;
            if (m_onResultsChanged) m_onResultsChanged();
            return;
        }

        // Process next pending directory
        auto& pending = m_walker.pendingDirs.front();
        m_walker.pendingDirs.erase(m_walker.pendingDirs.begin());

        // Load directory children from cache or file system
        const DirectoryResult* cached = m_cache.Get(pending.path);
        DirectoryResult listed;
        if (cached)
        {
            listed = cached->result; // copy
        }
        else
        {
            listed = m_fileSystem.ListDirectory(pending.path);
            if (!listed.success)
            {
                // Directory not found or error - skip
                continue;
            }
            // Cache the result
            m_cache.Put(pending.path, std::move(listed));
            listed = m_cache.Get(pending.path)->result;
        }

        // Scan entries
        for (const auto& entry : listed.entries)
        {
            // Check depth limit
            if (pending.depth >= kMaxDepth) { continue; }

            // Name match check (quick filter)
            if (!NaturalLess(entry.name, m_walker.query) &&
                !NaturalLess(m_walker.query, entry.name))
            {
                // Name matches query (case-insensitive substring or natural)
                // De-duplicate
                bool already = false;
                for (const auto& existing : m_matches)
                {
                    if (existing.fullPath == entry.fullPath)
                    {
                        already = true;
                        break;
                    }
                }
                if (!already && static_cast<size_t>(m_matches.size()) < kMaxMatches)
                {
                    m_matches.push_back(entry);
                }
            }

            // Add subdirectories to pending queue
            if (entry.IsDirectory() && !entry.IsHidden())
            {
                m_walker.pendingDirs.push_back({ entry.fullPath, static_cast<int>(pending.depth + 1) });
            }
        }

        m_walker.dirsScanned++;

        // Check frame budget
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed >= kFrameBudgetMs)
        {
            moreWork = true;
            break;
        }

    } while (!m_walker.pendingDirs.empty());

    if (moreWork)
    {
        // Will be called again next frame
        if (m_onResultsChanged) m_onResultsChanged();
    }
    else
    {
        // Search complete
        m_walker.active = false;
        if (m_onResultsChanged) m_onResultsChanged();
    }
}

void SearchController::CancelDeepSearch() noexcept
{
    m_walker.active = false;
    m_walker.generation++;
    m_matches.clear();
    if (m_onResultsChanged) m_onResultsChanged();
}

size_t SearchController::GetMatchCount() const noexcept
{
    return m_matches.size();
}

const std::vector<FileSystem::FileEntry>& SearchController::GetMatches() const noexcept
{
    return m_matches;
}

// ============================================================================
//  Shared helpers (anonymous namespace)
// ============================================================================

namespace {

[[nodiscard]] bool NaturalLess(const std::wstring& a, const std::wstring& b) noexcept
{
    // Case-insensitive comparison
    return _wcsicmp(a.c_str(), b.c_str()) < 0;
}

} // namespace

} // namespace DragonOS::Explorer2