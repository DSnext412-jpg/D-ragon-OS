#include <Explorer2/LoadPipeline.hpp>
#include <Explorer2/DirectoryCache.hpp>

namespace DragonOS::Explorer2 {

// ============================================================================
//  Request lifecycle
// ============================================================================

void LoadPipeline::Request(const std::wstring& path, LoadedCallback onLoaded) noexcept
{
    ++m_requestToken;
    m_pendingPath = path;
    m_pendingCallback = std::move(onLoaded);
}

void LoadPipeline::Cancel() noexcept
{
    ++m_requestToken;          // Invalidate any in-flight callback token.
    m_pendingPath.clear();
    m_pendingCallback = {};
    m_inFlight = false;
}

void LoadPipeline::Update(FileSystem::FileSystemService& fs) noexcept
{
    if (m_inFlight || m_pendingPath.empty())
    {
        return;
    }

    const std::wstring path = m_pendingPath;
    const uint64_t token = m_requestToken;

    // Fresh cache hit?  Apply immediately without touching the file system.
    if (const auto* cached = m_cache.Get(path))
    {
        m_pendingPath.clear();
        LoadedCallback callback = std::move(m_pendingCallback);
        m_pendingCallback = {};
        if (callback && token == m_requestToken)
        {
            callback(*cached);
        }
        return;
    }

    m_inFlight = true;
    fs.ListDirectoryAsync(path,
        [this, path, token](const FileSystem::DirectoryResult& result) noexcept
        {
            m_inFlight = false;

            // A newer request replaced this one while it was in flight.
            if (token != m_requestToken)
            {
                return;
            }

            if (result.success)
            {
                m_cache.Put(path, result);
            }

            m_pendingPath.clear();
            LoadedCallback callback = std::move(m_pendingCallback);
            m_pendingCallback = {};
            if (callback)
            {
                callback(result);
            }
        });
}

} // namespace DragonOS::Explorer2
