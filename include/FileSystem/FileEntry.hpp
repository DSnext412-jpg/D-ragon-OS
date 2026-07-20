#pragma once

#include <cstdint>
#include <string>

namespace DragonOS::FileSystem {

enum class FileAttributes : uint32_t {
    None        = 0x0,
    ReadOnly    = 0x1,
    Hidden      = 0x2,
    System      = 0x4,
    Directory   = 0x10,
    Archive     = 0x20,
    Normal      = 0x80,
};

inline constexpr FileAttributes operator|(FileAttributes a, FileAttributes b) noexcept
{
    return static_cast<FileAttributes>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr bool HasAttribute(FileAttributes value, FileAttributes flag) noexcept
{
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

struct FileEntry final {
    std::wstring  name;
    std::wstring  fullPath;
    uint64_t      size{ 0 };
    uint64_t      lastModified{ 0 };
    FileAttributes attributes{ FileAttributes::None };

    [[nodiscard]] bool IsDirectory() const noexcept
    {
        return HasAttribute(attributes, FileAttributes::Directory);
    }

    [[nodiscard]] bool IsHidden() const noexcept
    {
        return HasAttribute(attributes, FileAttributes::Hidden);
    }

    [[nodiscard]] bool IsReadOnly() const noexcept
    {
        return HasAttribute(attributes, FileAttributes::ReadOnly);
    }
};

} // namespace DragonOS::FileSystem
