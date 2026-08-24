// ============================================================================
//  LoadPipeline.hpp — Frame-aware directory load scheduling.
//
//  Part 10 (performance / async): the pipeline serializes directory loads so
//  at most one enumeration is in flight per window, drops stale requests
//  (rapid navigation never applies outdated results) and exposes loading
//  state for the status bar.
//
//  FileSystemService::ListDirectoryAsync currently completes synchronously;
//  the pipeline is the single integration point where a threaded worker can
//  be introduced later without touching any UI code.
// ============================================================================

#pragma once

#include <FileSystem/FileEntry.hpp>
#include <FileSystem/FileSystemService.hpp>

#include <cstdint>
#include <functional>
#include <string>

namespace DragonOS::Explorer2 {

class DirectoryCache;

class LoadPipeline final {
public:
    using LoadedCallback = std::function<void(const FileSystem::DirectoryResult&)>;

    explicit LoadPipeline(DirectoryCache& cache) noexcept : m_cache(cache) {}

    /// Requests the listing of @p path.  Any pending request is replaced and
    /// its result will be discarded.  @p onLoaded fires once with a result
    /// whose success flag reflects both enumeration and staleness checks.
    void Request(const std::wstring& path, LoadedCallback onLoaded) noexcept;

    /// Drops any pending request (e.g. tab closed mid-load).
    void Cancel() noexcept;

    /// Drives the pipeline; call once per frame before rendering.
    void Update(FileSystem::FileSystemService& fs) noexcept;

    [[nodiscard]] bool IsBusy() const noexcept { return m_inFlight; }
    [[nodiscard]] bool HasPendingWork() const noexcept { return m_inFlight || !m_pendingPath.empty(); }
    [[nodiscard]] const std::wstring& GetPendingPath() const noexcept { return m_pendingPath; }

private:
    DirectoryCache& m_cache;

    std::wstring   m_pendingPath;
    LoadedCallback m_pendingCallback;
    uint64_t       m_requestToken{ 0 };
    bool           m_inFlight{ false };
};

} // namespace DragonOS::Explorer2
