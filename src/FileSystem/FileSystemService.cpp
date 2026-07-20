#include <FileSystem/FileSystemService.hpp>

#include <Engine/EngineContext.hpp>

#include <algorithm>
#include <deque>
#include <cwchar>
#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace DragonOS::FileSystem {

namespace {

uint64_t FileTimeToUint64(const FILETIME& ft) noexcept
{
    return (static_cast<uint64_t>(ft.dwHighDateTime) << 32) |
            static_cast<uint64_t>(ft.dwLowDateTime);
}

} // anonymous namespace

// ============================================================================
//  Engine::System
// ============================================================================

bool FileSystemService::Initialize(Engine::EngineContext& /*ctx*/) noexcept
{
    if (m_initialized) { return true; }
    m_initialized = true;
    return true;
}

void FileSystemService::Shutdown() noexcept
{
    m_initialized = false;
}

void FileSystemService::Update(float /*deltaTime*/) noexcept
{
}

void FileSystemService::Render(Engine::EngineContext& /*ctx*/) noexcept
{
}

void FileSystemService::Resize(float /*width*/, float /*height*/) noexcept
{
}

// ============================================================================
//  Directory operations
// ============================================================================

DirectoryResult FileSystemService::ListDirectory(const std::wstring& path) noexcept
{
    DirectoryResult result;

    if (path.empty())
    {
        result.success = false;
        result.errorMessage = L"Path is empty";
        return result;
    }

    WIN32_FIND_DATAW findData;
    const std::wstring searchPath = path + L"\\*";

    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        result.success = false;
        result.errorMessage = L"Failed to open directory";
        return result;
    }

    do {
        // Skip . and ..
        if (wcscmp(findData.cFileName, L".") == 0 ||
            wcscmp(findData.cFileName, L"..") == 0)
        {
            continue;
        }

        FileEntry entry;
        entry.name = findData.cFileName;
        entry.fullPath = Combine(path, entry.name);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            entry.attributes = entry.attributes | FileAttributes::Directory;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        {
            entry.attributes = entry.attributes | FileAttributes::Hidden;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            entry.attributes = entry.attributes | FileAttributes::ReadOnly;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        {
            entry.attributes = entry.attributes | FileAttributes::System;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
        {
            entry.attributes = entry.attributes | FileAttributes::Archive;
        }

        entry.size = (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) |
                      static_cast<uint64_t>(findData.nFileSizeLow);

        entry.lastModified = FileTimeToUint64(findData.ftLastWriteTime);

        result.entries.push_back(std::move(entry));
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    // Sort: directories first, then files, alphabetically
    std::sort(result.entries.begin(), result.entries.end(),
        [](const FileEntry& a, const FileEntry& b) {
            if (a.IsDirectory() != b.IsDirectory())
            {
                return a.IsDirectory() > b.IsDirectory();
            }
            return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
        });

    result.success = true;
    return result;
}

void FileSystemService::ListDirectoryAsync(
    const std::wstring& path,
    AsyncCallback       callback) noexcept
{
    // Execute synchronously for now; future: thread pool
    DirectoryResult result = ListDirectory(path);
    if (callback)
    {
        callback(result);
    }
}

std::vector<std::wstring> FileSystemService::GetLogicalDrives() noexcept
{
    std::vector<std::wstring> drives;

    const DWORD mask = ::GetLogicalDrives();
    if (mask == 0) { return drives; }

    for (int i = 0; i < 26; ++i)
    {
        if (mask & (1 << i))
        {
            wchar_t drive[] = { static_cast<wchar_t>(L'A' + i), L':', L'\\', L'\0' };
            drives.push_back(drive);
        }
    }

    return drives;
}

// ============================================================================
//  File operations
// ============================================================================

FileEntry FileSystemService::GetFileInfo(const std::wstring& path) noexcept
{
    FileEntry entry;
    entry.fullPath = path;
    entry.name = GetFileName(path);

    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr))
    {
        return entry;
    }

    if (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        entry.attributes = entry.attributes | FileAttributes::Directory;
    if (attr.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        entry.attributes = entry.attributes | FileAttributes::Hidden;
    if (attr.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        entry.attributes = entry.attributes | FileAttributes::ReadOnly;

    entry.size = (static_cast<uint64_t>(attr.nFileSizeHigh) << 32) |
                  static_cast<uint64_t>(attr.nFileSizeLow);
    entry.lastModified = FileTimeToUint64(attr.ftLastWriteTime);

    return entry;
}

bool FileSystemService::Exists(const std::wstring& path) noexcept
{
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool FileSystemService::CreateFolder(const std::wstring& path) noexcept
{
    return ::CreateDirectoryW(path.c_str(), nullptr) != 0;
}

bool FileSystemService::EraseFile(const std::wstring& path) noexcept
{
    return ::DeleteFileW(path.c_str()) != 0;
}

bool FileSystemService::EraseDirectory(const std::wstring& path, bool recursive) noexcept
{
    if (recursive)
    {
        // Use SHFileOperation for recursive delete
        SHFILEOPSTRUCTW op{};
        std::wstring nullTerminated = path + L'\0';
        op.wFunc = FO_DELETE;
        op.pFrom = nullTerminated.c_str();
        op.fFlags = FOF_NO_UI | FOF_SILENT;
        return SHFileOperationW(&op) == 0;
    }
    return RemoveDirectoryW(path.c_str()) != 0;
}

bool FileSystemService::MoveItem(const std::wstring& src, const std::wstring& dest) noexcept
{
    return MoveFileW(src.c_str(), dest.c_str()) != 0;
}

bool FileSystemService::CopyItem(const std::wstring& src, const std::wstring& dest, bool overwrite) noexcept
{
    return CopyFileW(src.c_str(), dest.c_str(), !overwrite) != 0;
}

bool FileSystemService::RenameItem(const std::wstring& oldPath, const std::wstring& newPath) noexcept
{
    return MoveFileW(oldPath.c_str(), newPath.c_str()) != 0;
}

// ============================================================================
//  Path utilities
// ============================================================================

std::wstring FileSystemService::GetKnownFolderPath(KnownFolder folder) noexcept
{
    KNOWNFOLDERID rfid;
    switch (folder)
    {
    case KnownFolder::Home:        rfid = FOLDERID_Profile;     break;
    case KnownFolder::Desktop:     rfid = FOLDERID_Desktop;     break;
    case KnownFolder::Documents:   rfid = FOLDERID_Documents;   break;
    case KnownFolder::Downloads:   rfid = FOLDERID_Downloads;   break;
    case KnownFolder::Pictures:    rfid = FOLDERID_Pictures;    break;
    case KnownFolder::Music:       rfid = FOLDERID_Music;       break;
    case KnownFolder::Videos:      rfid = FOLDERID_Videos;      break;
    default:                       rfid = FOLDERID_Profile;     break;
    }

    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(rfid, 0, nullptr, &path) != S_OK)
    {
        if (path) { CoTaskMemFree(path); }
        return L"";
    }

    std::wstring result(path);
    CoTaskMemFree(path);
    return result;
}

std::wstring FileSystemService::GetParentPath(const std::wstring& path) noexcept
{
    if (path.empty()) { return L""; }

    std::wstring normalized = NormalizePath(path);
    if (normalized.empty()) { return L""; }

    // Handle root paths like C:\
    if (IsRootPath(normalized)) { return normalized; }

    // Remove trailing backslash if present
    if (normalized.back() == L'\\')
    {
        normalized.pop_back();
        if (IsRootPath(normalized)) { return normalized; }
    }

    auto pos = normalized.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        return L"";
    }

    // If we end up with something like "C:", add backslash
    std::wstring parent = normalized.substr(0, pos);
    if (parent.size() == 2 && parent[1] == L':')
    {
        parent += L'\\';
    }

    return parent;
}

std::wstring FileSystemService::GetFileName(const std::wstring& path) noexcept
{
    auto pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos)
    {
        return path;
    }
    return path.substr(pos + 1);
}

std::wstring FileSystemService::GetExtension(const std::wstring& path) noexcept
{
    auto name = GetFileName(path);
    auto pos = name.find_last_of(L'.');
    if (pos == std::wstring::npos || pos == 0)
    {
        return L"";
    }
    return name.substr(pos);
}

std::wstring FileSystemService::Combine(const std::wstring& a, const std::wstring& b) noexcept
{
    if (a.empty()) { return b; }
    if (b.empty()) { return a; }

    // If b is absolute, return b
    if (b.size() >= 2 && b[1] == L':') { return b; }
    if (b.size() >= 1 && b[0] == L'\\') { return b; }

    std::wstring result = a;
    if (result.back() != L'\\')
    {
        result += L'\\';
    }
    result += b;
    return result;
}

std::wstring FileSystemService::NormalizePath(const std::wstring& path) noexcept
{
    if (path.empty()) { return L""; }

    wchar_t buffer[MAX_PATH];
    DWORD len = GetFullPathNameW(path.c_str(), MAX_PATH, buffer, nullptr);
    if (len == 0 || len >= MAX_PATH)
    {
        return path;
    }
    return std::wstring(buffer, len);
}

bool FileSystemService::IsRootPath(const std::wstring& path) noexcept
{
    if (path.size() == 3 && path[1] == L':' && path[2] == L'\\')
    {
        return true;
    }
    if (path == L"\\")
    {
        return true;
    }
    return false;
}

// ============================================================================
//  Display helpers
// ============================================================================

std::wstring FileSystemService::FormatFileSize(uint64_t bytes) noexcept
{
    if (bytes == 0) { return L"0 bytes"; }

    const wchar_t* suffixes[] = { L"bytes", L"KB", L"MB", L"GB", L"TB" };
    int suffixIdx = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && suffixIdx < 4)
    {
        size /= 1024.0;
        ++suffixIdx;
    }

    wchar_t buffer[64];
    if (suffixIdx == 0)
    {
        swprintf_s(buffer, L"%.0f %s", size, suffixes[suffixIdx]);
    }
    else
    {
        swprintf_s(buffer, L"%.1f %s", size, suffixes[suffixIdx]);
    }

    return buffer;
}

std::wstring FileSystemService::FormatDateTime(uint64_t filetime) noexcept
{
    FILETIME ft;
    ft.dwLowDateTime  = static_cast<DWORD>(filetime & 0xFFFFFFFF);
    ft.dwHighDateTime = static_cast<DWORD>(filetime >> 32);

    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);

    wchar_t buffer[64];
    swprintf_s(buffer, L"%04d-%02d-%02d %02d:%02d",
               st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute);

    return buffer;
}

std::wstring FileSystemService::GetKnownFolderDisplayName(KnownFolder folder) noexcept
{
    switch (folder)
    {
    case KnownFolder::Home:        return L"Home";
    case KnownFolder::Desktop:     return L"Desktop";
    case KnownFolder::Documents:   return L"Documents";
    case KnownFolder::Downloads:   return L"Downloads";
    case KnownFolder::Pictures:    return L"Pictures";
    case KnownFolder::Music:       return L"Music";
    case KnownFolder::Videos:      return L"Videos";
    default:                       return L"";
    }
}

} // namespace DragonOS::FileSystem
