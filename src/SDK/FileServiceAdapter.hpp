#pragma once

#include <DragonOS/File.hpp>

#include <FileSystem/FileSystemService.hpp>

namespace DragonOS::SDK {

class FileServiceAdapter final : public dragonos::sdk::IFileService {
public:
    explicit FileServiceAdapter(
        FileSystem::FileSystemService& svc) noexcept
        : m_service{ svc }
    {
    }

    std::vector<dragonos::sdk::FileEntry> ListDirectory(
        std::wstring_view path) noexcept override;
    bool Exists(std::wstring_view path) noexcept override;
    bool CreateFolder(std::wstring_view path) noexcept override;
    bool DeleteFile(std::wstring_view path) noexcept override;
    bool DeleteDirectory(std::wstring_view path, bool recursive) noexcept override;
    std::wstring Combine(
        std::wstring_view a,
        std::wstring_view b) noexcept override;
    std::wstring GetParent(std::wstring_view path) noexcept override;
    std::wstring GetFileName(std::wstring_view path) noexcept override;

private:
    FileSystem::FileSystemService& m_service;
};

} // namespace DragonOS::SDK
