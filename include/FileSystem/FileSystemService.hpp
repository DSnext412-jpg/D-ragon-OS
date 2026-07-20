#pragma once

#include <FileSystem/FileEntry.hpp>

#include <Engine/System.hpp>

#include <functional>
#include <string>
#include <vector>

namespace DragonOS::FileSystem {

enum class KnownFolder {
    Home,
    Desktop,
    Documents,
    Downloads,
    Pictures,
    Music,
    Videos,
};

struct DirectoryResult final {
    bool                success{ false };
    std::wstring        errorMessage;
    std::vector<FileEntry> entries;
};

using AsyncCallback = std::function<void(const DirectoryResult&)>;

class FileSystemService final : public Engine::System {
public:
    FileSystemService() noexcept = default;
    ~FileSystemService() noexcept { Shutdown(); }

    FileSystemService(const FileSystemService&)            = delete;
    FileSystemService& operator=(const FileSystemService&) = delete;
    FileSystemService(FileSystemService&&)                 = delete;
    FileSystemService& operator=(FileSystemService&&)      = delete;

    // ── Engine::System ───────────────────────────────────────────────────

    bool Initialize(Engine::EngineContext& ctx) noexcept override;
    void Shutdown() noexcept override;
    void Update(float deltaTime) noexcept override;
    void Render(Engine::EngineContext& ctx) noexcept override;
    void Resize(float width, float height) noexcept override;

    // ── Directory operations ─────────────────────────────────────────────

    [[nodiscard]] DirectoryResult ListDirectory(const std::wstring& path) noexcept;

    void ListDirectoryAsync(
        const std::wstring& path,
        AsyncCallback       callback) noexcept;

    [[nodiscard]] std::vector<std::wstring> GetLogicalDrives() noexcept;

    // ── File operations ──────────────────────────────────────────────────

    [[nodiscard]] FileEntry GetFileInfo(const std::wstring& path) noexcept;
    [[nodiscard]] bool Exists(const std::wstring& path) noexcept;
    [[nodiscard]] bool CreateFolder(const std::wstring& path) noexcept;
    [[nodiscard]] bool EraseFile(const std::wstring& path) noexcept;
    [[nodiscard]] bool EraseDirectory(const std::wstring& path, bool recursive) noexcept;
    [[nodiscard]] bool MoveItem(const std::wstring& src, const std::wstring& dest) noexcept;
    [[nodiscard]] bool CopyItem(const std::wstring& src, const std::wstring& dest, bool overwrite) noexcept;
    [[nodiscard]] bool RenameItem(const std::wstring& oldPath, const std::wstring& newPath) noexcept;

    // ── Path utilities ───────────────────────────────────────────────────

    [[nodiscard]] std::wstring GetKnownFolderPath(KnownFolder folder) noexcept;
    [[nodiscard]] static std::wstring GetParentPath(const std::wstring& path) noexcept;
    [[nodiscard]] static std::wstring GetFileName(const std::wstring& path) noexcept;
    [[nodiscard]] static std::wstring GetExtension(const std::wstring& path) noexcept;
    [[nodiscard]] static std::wstring Combine(const std::wstring& a, const std::wstring& b) noexcept;
    [[nodiscard]] static std::wstring NormalizePath(const std::wstring& path) noexcept;
    [[nodiscard]] static bool IsRootPath(const std::wstring& path) noexcept;

    // ── Display helpers ──────────────────────────────────────────────────

    [[nodiscard]] static std::wstring FormatFileSize(uint64_t bytes) noexcept;
    [[nodiscard]] static std::wstring FormatDateTime(uint64_t filetime) noexcept;

    // ── Known folder display names ───────────────────────────────────────

    [[nodiscard]] static std::wstring GetKnownFolderDisplayName(KnownFolder folder) noexcept;

private:
    bool m_initialized{ false };
};

} // namespace DragonOS::FileSystem
