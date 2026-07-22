#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#ifdef DeleteFile
#undef DeleteFile
#endif

namespace dragonos::sdk {

struct FileEntry {
    std::wstring name;
    std::wstring fullPath;
    uint64_t size{ 0 };
    bool isDirectory{ false };
    bool isHidden{ false };
};

class IFileService {
public:
    virtual ~IFileService() noexcept = default;

    virtual std::vector<FileEntry> ListDirectory(std::wstring_view path) noexcept = 0;
    virtual bool Exists(std::wstring_view path) noexcept = 0;
    virtual bool CreateFolder(std::wstring_view path) noexcept = 0;
    virtual bool DeleteFile(std::wstring_view path) noexcept = 0;
    virtual bool DeleteDirectory(std::wstring_view path, bool recursive) noexcept = 0;
    virtual std::wstring Combine(std::wstring_view a, std::wstring_view b) noexcept = 0;
    virtual std::wstring GetParent(std::wstring_view path) noexcept = 0;
    virtual std::wstring GetFileName(std::wstring_view path) noexcept = 0;
};

} // namespace dragonos::sdk
