#include "FileServiceAdapter.hpp"

namespace DragonOS::SDK {

std::vector<dragonos::sdk::FileEntry> FileServiceAdapter::ListDirectory(
    std::wstring_view path) noexcept
{
    const auto result = m_service.ListDirectory(std::wstring{ path });
    std::vector<dragonos::sdk::FileEntry> entries;
    entries.reserve(result.entries.size());

    for (const auto& fe : result.entries)
    {
        dragonos::sdk::FileEntry entry;
        entry.name = fe.name;
        entry.fullPath = fe.fullPath;
        entry.size = fe.size;
        entry.isDirectory = fe.IsDirectory();
        entries.push_back(std::move(entry));
    }

    return entries;
}

bool FileServiceAdapter::Exists(std::wstring_view path) noexcept
{
    return m_service.Exists(std::wstring{ path });
}

bool FileServiceAdapter::CreateFolder(std::wstring_view path) noexcept
{
    return m_service.CreateFolder(std::wstring{ path });
}

bool FileServiceAdapter::DeleteFile(std::wstring_view path) noexcept
{
    return m_service.EraseFile(std::wstring{ path });
}

bool FileServiceAdapter::DeleteDirectory(std::wstring_view path, bool recursive) noexcept
{
    return m_service.EraseDirectory(std::wstring{ path }, recursive);
}

std::wstring FileServiceAdapter::Combine(
    std::wstring_view a, std::wstring_view b) noexcept
{
    return FileSystem::FileSystemService::Combine(
        std::wstring{ a }, std::wstring{ b });
}

std::wstring FileServiceAdapter::GetParent(std::wstring_view path) noexcept
{
    return FileSystem::FileSystemService::GetParentPath(std::wstring{ path });
}

std::wstring FileServiceAdapter::GetFileName(std::wstring_view path) noexcept
{
    return FileSystem::FileSystemService::GetFileName(std::wstring{ path });
}

} // namespace DragonOS::SDK
